#include "RangeIterator.h"
#include "modules/modules.h"
#include <cowlang/Callable.h>
#include <cowlang/CallableVMFunction.h>
#include <cowlang/Dictionary.h>
#include <modules/blockchain_module.h>

#include <cowlang/InterpreterTypes.h>
#include <cowlang/List.h>
#include <cowlang/PersistableDictionary.h>
#include <cowlang/Scope.h>

#include <iostream>
#include <map>
#include <memory>
#include <sstream>

bool devmode = false;
bool contractmode = false;


namespace cow
{


Interpreter::Interpreter(const bitstream &data, MemoryManager &mem)
: m_mem(mem), m_num_execution_steps(0), m_execution_step_limit(0), m_data(data)
{
    do_not_free_scope = false;
    m_global_scope = new(memory_manager()) Scope(memory_manager());

    // add persistent store to interpreter
    store = std::make_shared<PersistableDictionary>(mem);
    m_global_scope->set_value("store", store);
}

Interpreter::Interpreter(const bitstream &data, MemoryManager &mem, Interpreter &scope_borrower)
: m_mem(mem), m_num_execution_steps(0), m_execution_step_limit(0), m_data(data)
{
    store = scope_borrower.get_storage_pointer();
    do_not_free_scope = true;
    m_global_scope = scope_borrower.m_global_scope;
}

Interpreter::~Interpreter()
{
    if(!do_not_free_scope)
        delete m_global_scope;

    // no need to free store: shared pointer will release it properly
}


ValuePtr Interpreter::read_function_stub(Interpreter &inti)
{

    LoopState dummy_loop_state = LoopState::None;

    uint32_t total_stub_len;
    NodeType type;


    m_data >> total_stub_len;
    m_data >> type;

    if(type == NodeType::FunctionStart)
    {

        // cycle until we find FunctionStartDefaults (arg parsing)
        uint32_t num_args = 0;
        m_data >> num_args;

        std::vector<std::string> args;
        std::vector<ValuePtr> defaults;

        for(uint32_t i = 0; i < num_args; ++i)
        {
            CHARGE_EXECUTION;
            auto arg = read_name();
            args.push_back(arg);
        }

        m_data >> type;
        if(type != NodeType::FunctionStartDefaults)
        {
            throw std::runtime_error("Stop hacking the bytecode, you pathetic little worm!");
        }

        // cycle until we find FunctionStartDefaults (arg parsing)
        uint32_t num_defaults = 0;
        m_data >> num_defaults;

        if(num_defaults != num_args)
        {
            throw std::runtime_error("Stop hacking the bytecode, you pathetic little worm!");
        }

        bool non_standard = false;
        for(uint32_t i = 0; i < num_defaults; ++i)
        {
            CHARGE_EXECUTION;
            auto arg = execute_next(inti.get_scope(), dummy_loop_state);
            if(arg != nullptr)
                non_standard = true;
            if(arg == nullptr && non_standard)
            {
                throw std::runtime_error("Non-default argument follows default argument");
            }
            defaults.push_back(arg);
        }

        m_data >> type;
        if(type != NodeType::FunctionStartStub)
        {
            throw std::runtime_error("Stop hacking the bytecode, you pathetic little worm!");
        }


        std::string stub = m_data.Read(total_stub_len);
        bitstream innerfunction(stub);

        m_data >> type;

        if(type != NodeType::FunctionEnd)
        {
            throw std::runtime_error(
            "Are you kidding me, you stupid script kiddy? [wrong type order]");
        }

        ValuePtr pcl = wrap_value<CallableVMFunction>(
        new(memory_manager()) CallableVMFunction(memory_manager(), innerfunction, args, defaults));
        return pcl;
    }
    else
        throw std::runtime_error("Are you kidding me, you stupid script kiddy? [coder is insane]");
}

void Interpreter::re_assign_bitstream(const bitstream &data)
{
    const std::string raw = data.store();
    m_data.assign((uint8_t *)raw.data(), raw.size());
}

ModulePtr Interpreter::get_module(const std::string &name)
{
    auto it = m_loaded_modules.find(name);

    if(it != m_loaded_modules.end())
    {
        return it->second;
    }

    std::shared_ptr<Module> module = nullptr;

    if(name == "rand")
    {
        module = wrap_value<Module>(new(memory_manager()) RandModule(memory_manager()));
    }
#ifdef USE_GEO
    else if(name == "geo")
    {
        module = wrap_value<Module>(new(memory_manager()) GeoModule(memory_manager()));
    }
#endif
    else
        return nullptr;

    m_loaded_modules.emplace(name, module);
    return module;
}

void Interpreter::load_from_module(Scope &scope, const std::string &mname, const std::string &name, const std::string &as_name)
{
    auto module = get_module(mname);

    if(!module)
    {
        throw std::runtime_error("Unknown module: " + mname);
    }

    scope.set_value(as_name == "" ? name : as_name, module->get_member(name));
}

const uint32_t Interpreter::num_execution_steps() const { return m_num_execution_steps; }
const uint32_t Interpreter::max_execution_steps() const { return m_execution_step_limit; }
const uint32_t Interpreter::num_mem() const { return m_mem.get_mem(); }
const uint32_t Interpreter::max_mem() const { return m_mem.get_max_mem(); }

void Interpreter::set_execution_step_limit(uint32_t limit)
{
    if(limit == 0)
    {
        throw std::runtime_error("Limit has to be larger than 0");
    }

    m_execution_step_limit = limit;
}

void Interpreter::set_num_execution_steps(uint32_t current) { m_num_execution_steps = current; }

void Interpreter::load_module(Scope &scope, const std::string &mname, const std::string &as_name)
{
    auto module = get_module(mname);

    if(!module)
    {
        throw std::runtime_error("Unknown module: " + mname);
    }

    scope.set_value(as_name == "" ? mname : as_name, module);
}

ValuePtr Interpreter::execute()
{
    LoopState loop_state = LoopState::None;
    ValuePtr val = execute_next(*m_global_scope, loop_state);
    return val;
}

ValuePtr Interpreter::execute_in_scope(Scope &scope)
{
    LoopState loop_state = LoopState::None;
    ValuePtr val = execute_next(scope, loop_state);
    return val;
}


std::vector<std::string> Interpreter::read_names()
{
    std::vector<std::string> result;

    NodeType type;
    m_data >> type;
    if(type == NodeType::Name)
    {
        std::string str;
        m_data >> str;
        result.push_back(str);
    }
    else if(type == NodeType::String)
    {
        std::string str;
        m_data >> str;
        result.push_back(str);
    }
    else if(type == NodeType::Tuple)
    {
        uint32_t num_elems = 0;
        m_data >> num_elems;

        if(num_elems != 2)
            throw std::runtime_error("Can only name pairs");

        result.push_back(read_name());
        result.push_back(read_name());
    }
    else
    {
        throw std::runtime_error("Not a valid name [" + std::to_string((int)type) + "]");
    }
    return result;
}


std::string Interpreter::read_name()
{
    NodeType type;
    m_data >> type;
    if(type == NodeType::Name)
    {
        std::string str;
        m_data >> str;
        return str;
    }
    else if(type == NodeType::String)
    {
        std::string str;
        m_data >> str;
        return str;
    }
    else
        throw std::runtime_error("Not a valid name");
}


ValuePtr Interpreter::execute_next(Scope &scope, LoopState &loop_state)
{

    CHARGE_EXECUTION;

    auto start = m_data.pos();
    ValuePtr returnval = nullptr;

    NodeType type;
    m_data >> type;


    LoopState dummy_loop_state = LoopState::None;

    switch(type)
    {
    case NodeType::ImportFrom:
    {
        auto module = read_name();

        auto val = execute_next(scope, dummy_loop_state);
        ASSERT_GENERIC(val);

        if(val->type() == ValueType::Tuple)
        {
            auto t = value_cast<Tuple>(val);

            for(uint32_t i = 0; i < t->size(); ++i)
            {
                auto alias = value_cast<Alias>(t->get(i));
                load_from_module(scope, module, alias->name(), alias->as_name());
            }
        }
        else
        {
            auto alias = value_cast<Alias>(val);
            load_from_module(scope, module, alias->name(), alias->as_name());
        }
        break;
    }
    case NodeType::Tuple:
    {
        uint32_t num_elems = 0;
        m_data >> num_elems;

        auto tuple = wrap_value(new(memory_manager()) Tuple(memory_manager()));
        for(uint32_t i = 0; i < num_elems; ++i)
        {
            CHARGE_EXECUTION;
            auto val = execute_next(scope, dummy_loop_state);
            ASSERT_GENERIC(val);
            tuple->append(val);
        }

        returnval = tuple;
        break;
    }

    case NodeType::Import:
    {
        auto val = execute_next(scope, dummy_loop_state);
        ASSERT_GENERIC(val);
        auto alias = value_cast<Alias>(val);
        load_module(scope, alias->name(), alias->as_name());
        break;
    }
    case NodeType::Alias:
    {
        std::string name, as_name;
        m_data >> name >> as_name;
        returnval = wrap_value(new(memory_manager()) Alias(memory_manager(), name, as_name));
        break;
    }
    case NodeType::Pass:
    {
        break;
    }
    case NodeType::Attribute:
    {
        ValuePtr value = execute_next(scope, dummy_loop_state);
        ASSERT_GENERIC(value);
        std::string name = read_name();
        returnval = value->get_member(name);
        break;
    }
    case NodeType::Name:
    {
        std::string str;
        m_data >> str;

        if(str == "False")
            returnval = memory_manager().create_boolean(false);
        else if(str == "True")
            returnval = memory_manager().create_boolean(true);
        else
            returnval = scope.get_value(str);
        break;
    }
    case NodeType::Continue:
    {
        if(loop_state == LoopState::None)
        {
            throw std::runtime_error("Not a loop");
        }

        loop_state = LoopState::Continue;
        break;
    }
    case NodeType::Break:
    {
        if(loop_state == LoopState::None)
        {
            throw std::runtime_error("Not a loop");
        }

        loop_state = LoopState::Break;
        break;
    }
    case NodeType::Assign:
    {
        auto val = execute_next(scope, dummy_loop_state);
        ASSERT_GENERIC(val);
        uint32_t num_targets = 0;
        m_data >> num_targets;


        for(uint32_t i = 0; i < num_targets; ++i)
        {
            CHARGE_EXECUTION;

            // here we need to decide whether we are dealing with a "subscript expression" or pure
            // constants for this, we use the "peek operator" '&'
            NodeType lookAhead;
            m_data &lookAhead;

            if(lookAhead == NodeType::Subscript)
            {
                // we found the subscript assignment, we need to remove the lookahead from m_data now! We do it quick and dirty
                m_data >> lookAhead;

                // Now we parse, and evaluate the subscript
                // first, we evaluate the index of the parent variable, which may be a complex expresion
                auto index = execute();
                ASSERT_GENERIC(index);
                // and now the subscript parent variable, which should always be a constant
                auto sscr = read_name();

                // Now we do the assigment and check for the validity of the index!
                // numeric for List and String for Dict!
                auto obj = scope.get_value(sscr);
                if(obj == nullptr)
                {
                    throw std::runtime_error("Array or dictionary '" + sscr + "' has not been defined.");
                }
                if(obj->type() == ValueType::Dictionary || obj->type() == ValueType::PersistableDictionary)
                {
                    if(index->type() != ValueType::String)
                    {
                        throw std::runtime_error("Dictionary indices must be of type 'String'");
                    }
                    if(obj->type() == ValueType::Dictionary)
                    {
                        std::shared_ptr<Dictionary> unwrapped = value_cast<Dictionary>(obj);
                        unwrapped->insert(index->str(), val);
                    }
                    else
                    {
                        std::shared_ptr<PersistableDictionary> unwrapped =
                        value_cast<PersistableDictionary>(obj);
                        unwrapped->insert(index->str(), val);
                    }
                }
                else if(obj->type() == ValueType::List)
                {
                    if(index->type() != ValueType::Integer)
                    {
                        throw std::runtime_error("Array indices must be of type 'Integer'");
                    }
                    std::shared_ptr<List> unwrapped = value_cast<List>(obj);
                    int64_t i = (int64_t)value_cast<IntVal>(index)->get();
                    unwrapped->set(i, val);
                }
                else
                {
                    throw std::runtime_error("Variable '" + sscr + "' is not an array or dictionary.");
                }
            }
            else
            {

                auto names = read_names();

                if(names.size() == 1)
                    scope.set_value(names[0], val);
                else if(names.size() == 2)
                {
                    if(val->type() == ValueType::Tuple)
                    {
                        auto t = value_cast<Tuple>(val);

                        scope.set_value(names[0], t->get(0));
                        scope.set_value(names[1], t->get(1));
                    }
                    else
                        throw std::runtime_error("cannot unpack value: not a tuple");
                }
                else if(names.size() == 3)
                {
                    auto dict = names[2];
                    auto subscript = names[1];
                    ValuePtr obj = scope.get_value(dict);
                    ASSERT_GENERIC(obj);
                    if(obj->type() == ValueType::Dictionary)
                    {
                        auto t = value_cast<Dictionary>(obj);
                        t->insert(subscript, val);
                    }
                    else if(obj->type() == ValueType::PersistableDictionary)
                    {
                        std::cout << "AAHAHAHAHAA " << dict << "{" << subscript << "}"
                                  << " PTR" << obj->type() << std::endl;

                        auto t = value_cast<PersistableDictionary>(obj);
                        t->insert(subscript, val);
                    }
                    else if(obj->type() == ValueType::List)
                    {
                        auto t = value_cast<List>(obj);
                    }
                    else
                    {
                        throw std::runtime_error(
                        "subscript assigning only supported by array and dict");
                    }
                }
                else
                {
                    throw std::runtime_error("invalid number of names");
                }
            }
        }

        break;
    }
    case NodeType::Global:
    {
        uint32_t size = 0;
        m_data >> size;
        for(uint32_t i = 0; i < size; ++i)
        {
            CHARGE_EXECUTION;

            auto name = read_name();
            scope.set_global_tag(name);
        }
        break;
    }
    case NodeType::StatementList:
    {
        uint32_t size = 0;
        m_data >> size;

        ValuePtr final = nullptr;

        for(uint32_t i = 0; i < size; ++i)
        {
            CHARGE_EXECUTION;

            if(scope.is_terminated() || loop_state == LoopState::Break || loop_state == LoopState::Continue)
            {
                skip_next();
            }
            else
            {
                LoopState child_loop_state = loop_state;
                if(child_loop_state == LoopState::TopLevel)
                {
                    child_loop_state = LoopState::Normal;
                }

                final = execute_next(scope, child_loop_state);

                if(loop_state != LoopState::None && child_loop_state != LoopState::Normal)
                {
                    loop_state = child_loop_state;
                }
            }
        }

        returnval = final;
        break;
    }
    case NodeType::UnaryOp:
    {
        UnaryOpType type;
        m_data >> type;

        auto res = execute_next(scope, dummy_loop_state);
        ASSERT_GENERIC(res);

        if(res->type() == ValueType::Bool)
        {
            bool cond = value_cast<BoolVal>(res)->get();

            switch(type)
            {
            case UnaryOpType::Not:
                returnval = memory_manager().create_boolean(!cond);
                break;
            default:
                throw std::runtime_error("Unknown unary operation");
            }
        }
        else if(res && res->type() == ValueType::Integer)
        {
            uint64_t i = value_cast<IntVal>(res)->get();

            switch(type)
            {
            case UnaryOpType::Sub:
                returnval = memory_manager().create_integer((-1) * i);
                break;
            case UnaryOpType::Add:
                returnval = res;
                break;
            case UnaryOpType::Not:
                returnval = memory_manager().create_boolean(false);
                break;
            default:
                throw std::runtime_error("Unknown unary operation");
            }
        }
        else
        {
            switch(type)
            {
            case UnaryOpType::Not:
                returnval = memory_manager().create_boolean(res == nullptr);
                break;
            default:
                throw std::runtime_error("Unknonw unary op");
            }
        }

        break;
    }
    case NodeType::BoolOp:
    {
        BoolOpType type;
        uint32_t num_vals = 0;
        m_data >> type >> num_vals;

        bool res;

        if(type == BoolOpType::And)
        {
            res = true;

            for(uint32_t i = 0; i < num_vals; ++i)
            {
                CHARGE_EXECUTION;
                if(!res)
                {
                    skip_next();
                    continue;
                }

                auto val = execute_next(scope, dummy_loop_state);

                if(!val)
                {
                    res = false;
                    continue;
                }

                if(val->type() != ValueType::Bool)
                    throw std::runtime_error("not a valid bool operation");

                if(!value_cast<BoolVal>(val)->get())
                    res = false;
            }
        }
        else if(type == BoolOpType::Or)
        {
            res = false;

            for(uint32_t i = 0; i < num_vals; ++i)
            {
                CHARGE_EXECUTION;
                if(res)
                {
                    skip_next();
                    continue;
                }

                auto val = execute_next(scope, dummy_loop_state);

                if(!val)
                    continue;

                if(val->type() != ValueType::Bool)
                    throw std::runtime_error("not a valid bool operation");

                // FIXME use bool testable
                if(value_cast<BoolVal>(val)->get())
                    res = true;
            }
        }
        else
            throw std::runtime_error("unknown bool op type");

        returnval = memory_manager().create_boolean(res);
        break;
    }
    case NodeType::BinaryOp:
    {
        BinaryOpType type;
        m_data >> type;

        auto left = execute_next(scope, dummy_loop_state);
        auto right = execute_next(scope, dummy_loop_state);

        switch(type)
        {
        case BinaryOpType::Add:
        {
            ASSERT_LEFT_AND_RIGHT;
            if(left->type() == ValueType::Integer && right->type() == ValueType::Integer)
            {
                auto i1 = value_cast<IntVal>(left)->get();
                auto i2 = value_cast<IntVal>(right)->get();

                returnval = memory_manager().create_integer(i1 + i2);
            }
            else if(left->type() == ValueType::String && right->type() == ValueType::String)
            {
                auto s1 = value_cast<StringVal>(left)->get();
                auto s2 = value_cast<StringVal>(right)->get();

                returnval = memory_manager().create_string(s1 + s2);
            }
            else
            {
                std::stringstream sstr;
                sstr << "failed to add: incompatible types " << left->type() << right->type();
                throw std::runtime_error(sstr.str());
            }

            break;
        }
        case BinaryOpType::Mult:
        {
            ASSERT_LEFT_AND_RIGHT;
            if(left->type() == ValueType::Integer && right->type() == ValueType::Integer)
            {
                auto i1 = value_cast<IntVal>(left)->get();
                auto i2 = value_cast<IntVal>(right)->get();

                returnval = memory_manager().create_integer(i1 * i2);
            }
            else
                throw std::runtime_error("failed to multiply");

            break;
        }
        case BinaryOpType::Div:
        {
            ASSERT_LEFT_AND_RIGHT;
            if(left->type() == ValueType::Integer && right->type() == ValueType::Integer)
            {
                auto i1 = value_cast<IntVal>(left)->get();
                auto i2 = value_cast<IntVal>(right)->get();
                if(i2 == 0)
                    throw std::runtime_error("division by zero");

                returnval = memory_manager().create_integer(i1 / i2);
            }
            else if(left->type() == ValueType::Float && right->type() == ValueType::Float)
            {
                auto f1 = value_cast<FloatVal>(left)->get();
                auto f2 = value_cast<FloatVal>(right)->get();
                if(f2 == 0)
                    throw std::runtime_error("division by zero");
                returnval = memory_manager().create_float(f1 / f2);
            }
            else
                throw std::runtime_error("failed to multiply");

            break;
        }
        case BinaryOpType::Mod:
        {
            ASSERT_LEFT_AND_RIGHT;
            if(left->type() == ValueType::Integer && right->type() == ValueType::Integer)
            {
                auto i1 = value_cast<IntVal>(left)->get();
                auto i2 = value_cast<IntVal>(right)->get();
                if(i2 == 0)
                    throw std::runtime_error("modulus by zero");
                returnval = memory_manager().create_integer(i1 % i2);
            }
            else
                throw std::runtime_error("failed apply mod function");
            break;
        }

        case BinaryOpType::Sub:
        {
            ASSERT_LEFT_AND_RIGHT;

            if(left->type() == ValueType::Integer && right->type() == ValueType::Integer)
            {
                auto i1 = value_cast<IntVal>(left)->get();
                auto i2 = value_cast<IntVal>(right)->get();

                returnval = memory_manager().create_integer(i1 - i2);
            }
            else
                throw std::runtime_error("failed to sub");
            break;
        }
        default:
            throw std::runtime_error("Unknown binary operation");
        }

        break;
    }
    case NodeType::Return:
    {
        returnval = execute_next(scope, dummy_loop_state);
        scope.terminate();
        break;
    }
    case NodeType::List:
    {
        auto list = memory_manager().create_list();
        uint32_t size = 0;
        m_data >> size;

        for(uint32_t i = 0; i < size; ++i)
        {
            CHARGE_EXECUTION;
            auto res = execute_next(scope, dummy_loop_state);
            ASSERT_GENERIC(res);
            list->append(res);
        }

        returnval = list;
        break;
    }
    case NodeType::String:
    {
        std::string str;
        m_data >> str;

        returnval = memory_manager().create_string(str);
        break;
    }
    case NodeType::Compare:
    {
        auto current = execute_next(scope, dummy_loop_state);

        uint32_t size = 0;
        m_data >> size;

        for(uint32_t i = 0; i < size; ++i)
        {
            CHARGE_EXECUTION;
            CompareOpType op_type;
            m_data >> op_type;

            ValuePtr rval = execute_next(scope, dummy_loop_state);
            bool res = false;

            if(op_type == CompareOpType::Equals)
            {
                if(!current || !rval)
                    res = current == rval; // check if both are nullptrs
                else
                    res = (*current == *rval);
            }
            else if(op_type == CompareOpType::MoreEqual)
            {
                if(!current || !rval)
                    res = current == rval; // check if both are nullptrs
                else
                    res = (*current >= *rval);
            }
            else if(op_type == CompareOpType::More)
            {
                if(!current || !rval)
                    res = false; // FIXME throw exception
                else
                    res = (*current > *rval);
            }
            else if(op_type == CompareOpType::In)
            {
                ASSERT_GENERIC(rval);
                if(rval->type() != ValueType::List)
                    throw std::runtime_error("Can only call in on lists");
                ASSERT_GENERIC(current);
                res = value_cast<List>(rval)->contains(*current);
            }
            else if(op_type == CompareOpType::NotEqual)
            {
                if(!current || !rval)
                    res = current != rval;
                else
                    res = !(*current == *rval);
            }
            else if(op_type == CompareOpType::NotIn)
            {
                ASSERT_GENERIC(rval);
                if(rval->type() != ValueType::List)
                    throw std::runtime_error("Can only call in on lists");
                ASSERT_GENERIC(current);
                res = !value_cast<List>(rval)->contains(*current);
            }
            else if(op_type == CompareOpType::LessEqual)
            {
                if(!current || !rval)
                    res = current == rval;
                else
                    res = (*rval >= *current);
            }
            else if(op_type == CompareOpType::Less)
            {
                if(!current || !rval)
                    res = false;
                else
                    res = (*rval > *current);
            }
            else
                throw std::runtime_error("Unknown op type");

            current = memory_manager().create_boolean(res);
        }

        returnval = current;
        break;
    }
    case NodeType::Index:
    {
        returnval = execute_next(scope, dummy_loop_state);
        break;
    }
    case NodeType::Integer:
    {
        int32_t val;
        m_data >> val;
        returnval = memory_manager().create_integer(val);
        break;
    }
    case NodeType::Call:
    {
        auto callable = execute_next(scope, dummy_loop_state);
        ASSERT_GENERIC(callable);
        if(!callable->is_callable())
        {
            throw std::runtime_error("Cannot call un-callable!");
        }

        uint32_t num_args = 0;
        m_data >> num_args;

        std::vector<ValuePtr> args;
        for(uint32_t i = 0; i < num_args; ++i)
        {
            CHARGE_EXECUTION;
            auto arg = execute_next(scope, dummy_loop_state);
            args.push_back(arg);
        }

        uint32_t current_num = num_execution_steps();
        uint32_t current_max = max_execution_steps();
        try
        {
            returnval = value_cast<Callable>(callable)->call(args, scope, current_num, current_max);
        }
        catch(...)
        {
            set_num_execution_steps(current_num);
            throw;
        }

        break;
    }
    case NodeType::If:
    {
        auto test = execute_next(scope, loop_state);
        bool cond = test && test->bool_test();

        ValuePtr res = nullptr;

        if(cond)
        {
            res = execute_next(scope, loop_state);
        }
        else
        {
            skip_next();
        }

        returnval = res;
        break;
    }
    case NodeType::IfElse:
    {
        auto test = execute_next(scope, loop_state);
        if(test && test->type() != ValueType::Bool)
        {
            throw std::runtime_error("not a boolean!");
        }

        bool cond = test && value_cast<BoolVal>(test)->get();

        if(cond)
        {
            returnval = execute_next(scope, loop_state);
            skip_next();
        }
        else
        {
            skip_next();
            returnval = execute_next(scope, loop_state);
        }

        break;
    }
    case NodeType::Dictionary:
    {
        auto res = memory_manager().create_dictionary();

        uint32_t size;
        m_data >> size;

        for(uint32_t i = 0; i < size; ++i)
        {
            CHARGE_EXECUTION;
            auto name = read_name();
            auto value = execute_next(scope, dummy_loop_state);
            ASSERT_GENERIC(value);

            res->insert(name, value);
        }

        returnval = res;
        break;
    }
    case NodeType::Subscript:
    {
        auto slice = execute_next(scope, dummy_loop_state);
        auto val = execute_next(scope, dummy_loop_state);
        ASSERT_GENERIC(slice);
        ASSERT_GENERIC(val);
        if(val->type() == ValueType::Dictionary && slice->type() == ValueType::String)
        {
            returnval = value_cast<Dictionary>(val)->get(value_cast<StringVal>(slice)->get());
        }
        else if(val->type() == ValueType::PersistableDictionary && slice->type() == ValueType::String)
        {
            returnval = value_cast<PersistableDictionary>(val)->get(value_cast<StringVal>(slice)->get());
        }
        else if(val->type() == ValueType::List && slice->type() == ValueType::Integer)
        {
            returnval = value_cast<List>(val)->get(value_cast<IntVal>(slice)->get());
        }
        else if(val->type() == ValueType::Tuple && slice->type() == ValueType::Integer)
        {
            returnval = value_cast<Tuple>(val)->get(value_cast<IntVal>(slice)->get());
        }
        else
        {
            throw std::runtime_error("Invalid subscript");
        }

        break;
    }
    case NodeType::WhileLoop:
    {
        LoopState for_loop_state = LoopState::TopLevel;
        auto start = m_data.pos();

        while(!scope.is_terminated() && for_loop_state != LoopState::Break)
        {
            CHARGE_EXECUTION;
            m_data.move_to(start);

            auto test = execute_next(scope, dummy_loop_state);
            bool cond = test && test->bool_test();

            if(!cond)
            {
                break;
            }

            Scope body_scope(memory_manager(), scope);
            auto res = execute_next(body_scope, for_loop_state);

            // Propagate return?
            if(body_scope.is_terminated())
            {
                scope.terminate();
                returnval = res;
            }
        }

        skip_next();
        break;
    }
    case NodeType::ForLoop:
    {
        const std::vector<std::string> names = read_names();
        LoopState for_loop_state = LoopState::TopLevel;

        auto obj = execute_next(scope, dummy_loop_state);
        IteratorPtr iter = nullptr;

        ASSERT_GENERIC(obj);

        if(obj->is_generator())
        {
            iter = value_cast<Iterator>(obj);
        }
        else if(obj->can_iterate())
        {
            iter = value_cast<IterateableValue>(obj)->iterate();
        }
        else
        {
            throw std::runtime_error("Can't iterate");
        }

        while(!scope.is_terminated() && for_loop_state != LoopState::Break)
        {
            CHARGE_EXECUTION;
            Scope body_scope(memory_manager(), scope);
            ValuePtr next = nullptr;

            try
            {
                next = iter->next();
            }
            catch(stop_iteration_exception)
            {
                break;
            }

            if(names.size() == 1)
            {
                body_scope.set_value(names[0], next);
            }
            else if(names.size() == 2)
            {
                auto t = value_cast<Tuple>(next);
                if(!t)
                {
                    throw std::runtime_error("Cannot unpack values: not a tuple!");
                }

                body_scope.set_value(names[0], t->get(0));
                body_scope.set_value(names[1], t->get(1));
            }
            else
            {
                throw std::runtime_error("Cannot handle more than two names");
            }

            auto res = execute_next(body_scope, for_loop_state);

            // Propagate return?
            if(body_scope.is_terminated())
            {
                scope.terminate();
                returnval = res;
            }
        }

        skip_next();
        break;
    }
    case NodeType::ListComp:
    {
        auto body_pos = m_data.pos();
        skip_next();

        uint32_t num_loops = 0;
        m_data >> num_loops;

        if(num_loops != 1)
        {
            throw std::runtime_error("Only simple list comprehensions are supported");
        }

        NodeType type2;
        m_data >> type2;

        if(type2 != NodeType::Comprehension)
        {
            throw std::runtime_error("invalid type");
        }

        auto for_loop_state = LoopState::TopLevel;
        auto target = read_name();
        auto list = memory_manager().create_list();

        auto iter = value_cast<Iterator>(execute_next(scope, loop_state));
        ASSERT_GENERIC(iter);

        // no support for if statements yet
        skip_next();

        auto end_pos = m_data.pos();

        while(!scope.is_terminated() && for_loop_state != LoopState::Break)
        {
            CHARGE_EXECUTION;
            ValuePtr next;

            try
            {
                next = iter->next();
            }
            catch(stop_iteration_exception)
            {
                break;
            }

            Scope body_scope(memory_manager(), scope);
            body_scope.set_value(target, next);

            m_data.move_to(body_pos);
            auto res = execute_next(body_scope, for_loop_state);
            list->append(res);
        }

        m_data.move_to(end_pos);
        returnval = list;
        break;
    }
    case NodeType::AugmentedAssign:
    {
        BinaryOpType op_type;
        m_data >> op_type;

        // here we need to decide whether we are dealing with a "subscript expression" or pure
        // constants for this, we use the "peek operator" '&'
        NodeType lookAhead;
        m_data &lookAhead;

        if(lookAhead == NodeType::Subscript)
        {
            // we found the subscript assignment, we need to remove the lookahead from m_data now! We do it quick and dirty
            m_data >> lookAhead;

            // Now we parse, and evaluate the subscript
            // first, we evaluate the index of the parent variable, which may be a complex expresion
            auto index = execute();
            ASSERT_GENERIC(index);
            // and now the subscript parent variable, which should always be a constant
            auto sscr = read_name();

            // Now we do the assigment and check for the validity of the index!
            // numeric for List and String for Dict!
            auto obj = scope.get_value(sscr);
            auto val = execute_next(scope, dummy_loop_state);
            if(obj == nullptr)
            {
                throw std::runtime_error("Array or dictionary '" + sscr + "' has not been defined.");
            }
            if(obj->type() == ValueType::Dictionary || obj->type() == ValueType::PersistableDictionary)
            {
                if(index->type() != ValueType::String)
                {
                    throw std::runtime_error("Dictionary indices must be of type 'String'");
                }
                if(obj->type() == ValueType::Dictionary)
                {
                    std::shared_ptr<Dictionary> unwrapped = value_cast<Dictionary>(obj);
                    unwrapped->apply(index->str(), val, op_type);
                }
                else
                {
                    std::shared_ptr<PersistableDictionary> unwrapped =
                    value_cast<PersistableDictionary>(obj);
                    unwrapped->apply(index->str(), val, op_type);
                }
            }
            else if(obj->type() == ValueType::List)
            {
                if(index->type() != ValueType::Integer)
                {
                    throw std::runtime_error("Array indices must be of type 'Integer'");
                }
                std::shared_ptr<List> unwrapped = value_cast<List>(obj);
                int64_t i = (int64_t)value_cast<IntVal>(index)->get();
                unwrapped->apply(i, val, op_type);
            }
            else
            {
                throw std::runtime_error("Variable '" + sscr + "' is not an array or dictionary.");
            }
        }
        else
        {
            auto t_name = read_name();
            auto target = scope.get_value(t_name);

            auto value = execute_next(scope, dummy_loop_state);

            switch(op_type)
            {
            case BinaryOpType::Add:
            {
                if(!target || !value || target->type() != ValueType::Integer || value->type() != ValueType::Integer)
                {
                    throw std::runtime_error("Values need to be numerics");
                }

                auto i_target = value_cast<IntVal>(target);
                auto i_value = value_cast<IntVal>(value);
                i_target->set(i_target->get() + i_value->get());
                break;
            }
            case BinaryOpType::Sub:
            {
                if(!target || !value || target->type() != ValueType::Integer || value->type() != ValueType::Integer)
                {
                    throw std::runtime_error("Values need to be numerics");
                }

                auto i_target = value_cast<IntVal>(target);
                auto i_value = value_cast<IntVal>(value);
                i_target->set(i_target->get() - i_value->get());
                break;
            }
            case BinaryOpType::Mult:
            {
                if(!target || !value || target->type() != ValueType::Integer || value->type() != ValueType::Integer)
                {
                    throw std::runtime_error("Values need to be numerics");
                }

                auto i_target = value_cast<IntVal>(target);
                auto i_value = value_cast<IntVal>(value);
                i_target->set(i_target->get() * i_value->get());
                break;
            }
            default:
                throw std::runtime_error("Unknown binary op");
            }
            break;
        }
    }
    case NodeType::FunctionDef:
    {
        auto t_name = read_name();
        if(t_name.size() == 0)
            throw std::runtime_error("Function name of length zero");

        // Now, read the stub and get the stack jump point
        ValuePtr jump_point = read_function_stub(*this);
        ASSERT_GENERIC(jump_point);

        scope.set_value(t_name, jump_point);
        break;
    }
    default:
        throw std::runtime_error("Unknown node type!");
    }

    if(loop_state != LoopState::Normal && loop_state != LoopState::None)
        m_data.move_to(start);

    return returnval;
}

void Interpreter::skip_next()
{
    NodeType type;
    m_data >> type;

    switch(type)
    {
    case NodeType::Pass:
    {
        break;
    }
    case NodeType::Name:
    {
        std::string str;
        m_data >> str;
        break;
    }
    case NodeType::ForLoop:
    {
        read_names();
        skip_next();
        skip_next();
        break;
    }
    case NodeType::WhileLoop:
    {
        skip_next();
        skip_next();
        break;
    }
    case NodeType::Assign:
    {
        skip_next();

        uint32_t num_targets = 0;
        m_data >> num_targets;

        for(uint32_t i = 0; i < num_targets; ++i)
        {
            CHARGE_EXECUTION;
            skip_next();
        }
        break;
    }
    case NodeType::StatementList:
    {
        uint32_t size = 0;
        m_data >> size;

        for(uint32_t i = 0; i < size; ++i)
        {
            CHARGE_EXECUTION;
            skip_next();
        }
        break;
    }
    case NodeType::Index:
    case NodeType::Return:
    {
        skip_next();
        break;
    }
    case NodeType::String:
    {
        std::string str;
        m_data >> str;
        break;
    }
    case NodeType::Compare:
    {
        skip_next();
        uint32_t size = 0;
        m_data >> size;

        for(uint32_t i = 0; i < size; ++i)
        {
            CHARGE_EXECUTION;
            CompareOpType op_type;
            m_data >> op_type;
            skip_next();
        }
        break;
    }
    case NodeType::Integer:
    {
        int32_t val;
        m_data >> val;
        break;
    }
    case NodeType::Call:
    {
        skip_next();
        uint32_t num_args = 0;
        m_data >> num_args;
        for(uint32_t i = 0; i < num_args; ++i)
        {
            CHARGE_EXECUTION;
            skip_next();
        }
        break;
    }
    case NodeType::IfElse:
    {
        skip_next();
        skip_next();
        skip_next();
        break;
    }
    case NodeType::List:
    case NodeType::Tuple:
    {
        uint32_t size;
        m_data >> size;
        for(uint32_t i = 0; i < size; ++i)
        {
            CHARGE_EXECUTION;
            skip_next();
        }

        break;
    }
    case NodeType::Dictionary:
    {
        uint32_t size;
        m_data >> size;
        for(uint32_t i = 0; i < size; ++i)
        {
            CHARGE_EXECUTION;
            skip_next();
            skip_next();
        }
        break;
    }
    case NodeType::UnaryOp:
    {
        UnaryOpType type;
        m_data >> type;
        skip_next();
        break;
    }
    case NodeType::BoolOp:
    {
        BoolOpType op;
        uint32_t num_vals = 0;
        m_data >> op >> num_vals;

        for(uint32_t i = 0l; i < num_vals; ++i)
        {
            CHARGE_EXECUTION;
            skip_next();
        }
        break;
    }
    case NodeType::AugmentedAssign:
    case NodeType::BinaryOp:
    {
        BinaryOpType op;
        m_data >> op;
        skip_next();
        skip_next();
        break;
    }
    case NodeType::If:
    case NodeType::Attribute:
    case NodeType::Subscript:
    {
        skip_next();
        skip_next();
        break;
    }
    case NodeType::Break:
    case NodeType::Continue:
        break;

    default:
        throw std::runtime_error("Failed to skip unknown node type!");
    }
}

void Interpreter::set_module(const std::string &name, ModulePtr module)
{
    m_loaded_modules[name] = module;
}

void Interpreter::set_string(const std::string &name, const std::string &value)
{
    auto s = memory_manager().create_string(value);
    set_value(name, s);
}

void Interpreter::set_list(const std::string &name, const std::vector<std::string> &list)
{
    auto l = memory_manager().create_list();

    for(auto &e : list)
    {
        // FIXME support other types
        auto s = memory_manager().create_string(e);
        l->append(s);
    }

    set_value(name, l);
}

} // namespace cow
