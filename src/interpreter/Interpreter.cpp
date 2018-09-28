#include <memory>
#include <map>
#include <sstream>

#include <cowlang/Interpreter.h>
#include <cowlang/Callable.h>
#include <cowlang/Scope.h>

#include "RangeIterator.h"
#include "modules/modules.h"

namespace cow
{

enum class CompareOpType
{
    Undefined,
    Equals,
    In,
    Is,
    IsNot,
    Less,
    LessEqual,
    More,
    MoreEqual,
    NotEqual,
    NotIn,
};

enum class BoolOpType
{
    Undefined,
    And,
    Or
};

enum class BinaryOpType
{
    Undefined,
    Add,
    BitAnd,
    BitOr,
    BitXor,
    Div,
    FloorDiv,
    LeftShift,
    Mod,
    Mult,
    Power,
    RightShift,
    Sub
};

enum class UnaryOpType
{
    Undefined,
    Add,
    Invert,
    Not,
    Sub,
};

Interpreter::Interpreter(const bitstream &data, bool dummy_memory_manager)
    : m_num_execution_steps(0), m_execution_step_limit(0)
{
    if(dummy_memory_manager)
    {
        m_mem = new DummyMemoryManager();
    }
    else
    {
        m_mem = new DefaultMemoryManager();
    }

    m_global_scope = new (memory_manager()) Scope(memory_manager());
    m_data.assign(data.data(), data.size(), true);
}

Interpreter::~Interpreter()
{
    delete m_global_scope;
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
        module = wrap_value<Module>(new (memory_manager()) RandModule(memory_manager()));
    }
#ifdef USE_GEO
    else if(name == "geo")
    {
        module = wrap_value<Module>(new (memory_manager()) GeoModule(memory_manager()));
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

    scope.set_value(as_name == "" ? name: as_name, module->get_member(name));
}

uint32_t Interpreter::num_execution_steps() const
{
    return m_num_execution_steps;
}

void Interpreter::set_execution_step_limit(uint32_t limit)
{
    if(limit == 0)
    {
        throw std::runtime_error("Limit has to be larger than 0");
    }

    m_execution_step_limit = limit;
}

void Interpreter::load_module(Scope &scope, const std::string &mname, const std::string &as_name)
{
    auto module = get_module(mname);
        
    if(!module)
    {
        throw std::runtime_error("Unknown module: " + mname);
    }

    scope.set_value(as_name == "" ? mname: as_name, module);
}

ValuePtr Interpreter::execute()
{
    LoopState loop_state = LoopState::None;
    ValuePtr val = execute_next(*m_global_scope, loop_state);
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
        throw std::runtime_error("Not a valid name");

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
    m_num_execution_steps += 1;

    if(m_execution_step_limit > 0
       && m_num_execution_steps >= m_execution_step_limit)
    {
        throw execution_limit_exception("Reached maximum number of steps!");
    }

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

        auto tuple = wrap_value(new (memory_manager()) Tuple(memory_manager()));

        for(uint32_t i = 0; i < num_elems; ++i)
            tuple->append(execute_next(scope, dummy_loop_state));

        returnval = tuple;
        break;
    }
 
    case NodeType::Import:
    {
        auto val = execute_next(scope, dummy_loop_state);
        auto alias = value_cast<Alias>(val);
        load_module(scope, alias->name(), alias->as_name());
        break;
    }
    case NodeType::Alias:
    {
        std::string name, as_name;
        m_data >> name >> as_name;
        returnval = wrap_value(new (memory_manager()) Alias(memory_manager(), name, as_name));
        break;
    }
    case NodeType::Pass:
    {
        break;
    }
    case NodeType::Attribute:
    {
        ValuePtr value = execute_next(scope, dummy_loop_state);
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

        uint32_t num_targets = 0;
        m_data >> num_targets;

        for(uint32_t i = 0; i < num_targets; ++i)
        {
            auto names = read_names();

            if(names.size() == 1)
                scope.set_value(names[0], val);
            else if(names.size() == 2)
            {
                if(val->type() != ValueType::Tuple)
                {
                    throw std::runtime_error("cannot unpack value: not a tuple");
                }

                auto t = value_cast<Tuple>(val);

                scope.set_value(names[0], t->get(0));
                scope.set_value(names[1], t->get(1));
            }
            else
            {
                throw std::runtime_error("invalid number of names");
            }
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

        if(res && res->type() == ValueType::Bool)
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
            int32_t i = value_cast<IntVal>(res)->get();

            switch(type)
            {
            case UnaryOpType::Sub:
                returnval = memory_manager().create_integer((-1)*i);
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

                 //FIXME use bool testable
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
            #ifdef IS_ENCLAVE
                throw std::runtime_error("Failed to add");
            #else
                std::stringstream sstr;
                sstr << "failed to add: incompatible types " << left->type() << right->type();
                throw std::runtime_error(sstr.str());
            #endif
            }

            break;
        }
        case BinaryOpType::Mult:
        {
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
            if(left->type() == ValueType::Integer && right->type() == ValueType::Integer)
            {
                auto i1 = value_cast<IntVal>(left)->get();
                auto i2 = value_cast<IntVal>(right)->get();

                returnval = memory_manager().create_integer(i1 / i2);
            }
            else if(left->type() == ValueType::Float && right->type() == ValueType::Float)
            {
                auto f1 = value_cast<FloatVal>(left)->get();
                auto f2 = value_cast<FloatVal>(right)->get();

                returnval = memory_manager().create_float(f1 / f2);
            }
            else
                throw std::runtime_error("failed to multiply");

            break;
        } 
        case BinaryOpType::Mod:
        {
            if(left->type() == ValueType::Integer && right->type() == ValueType::Integer)
            {
                auto i1 = value_cast<IntVal>(left)->get();
                auto i2 = value_cast<IntVal>(right)->get();

                returnval = memory_manager().create_integer(i1 % i2);
            }
            else
                throw std::runtime_error("failed apply mod function");
            break;
        }
 
        case BinaryOpType::Sub:
        {
            if(!left || !right)
            {
                throw std::runtime_error("Cannot subtract on none values");
            }
            else if(left->type() == ValueType::Integer && right->type() == ValueType::Integer)
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
            auto res = execute_next(scope, dummy_loop_state);
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
                    res = false; //FIXME throw exception
                else
                    res = (*current > *rval);
            }
            else if(op_type == CompareOpType::In)
            {
                if(rval->type() != ValueType::List)
                    throw std::runtime_error("Can only call in on lists");

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
                if(rval->type() != ValueType::List)
                    throw std::runtime_error("Can only call in on lists");

                res = !value_cast<List>(rval)->contains(*current);
            }
            else if(op_type == CompareOpType::LessEqual)
            {
                if(!current || !rval)
                    res =  current == rval;
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
        if(!callable->is_callable())
        {
            throw std::runtime_error("Cannot call un-callable!");
        }

        uint32_t num_args = 0;
        m_data >> num_args;

        std::vector<ValuePtr> args;
        for(uint32_t i = 0; i < num_args; ++i)
        {
            auto arg = execute_next(scope, dummy_loop_state);
            args.push_back(arg);
        }

        returnval = value_cast<Callable>(callable)->call(args);
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
            auto name = read_name();
            auto value = execute_next(scope, dummy_loop_state);

            res->insert(name, value);
        }

        returnval = res;
        break;
    }
    case NodeType::Subscript:
    {
        auto slice = execute_next(scope, dummy_loop_state);
        auto val = execute_next(scope, dummy_loop_state);

        if(val->type() == ValueType::Dictionary && slice->type() == ValueType::String)
        {
            returnval = value_cast<Dictionary>(val)->get(value_cast<StringVal>(slice)->get());
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
            Scope body_scope(memory_manager(), scope);
            ValuePtr next = nullptr;

            try {
                next = iter->next();
            } catch(stop_iteration_exception) {
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

        // no support for if statements yet 
        skip_next();

        auto end_pos = m_data.pos();

        while(!scope.is_terminated() && for_loop_state != LoopState::Break)
        {
            ValuePtr next;

            try {
                next = iter->next();
            } catch(stop_iteration_exception) {
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
            auto i_value  = value_cast<IntVal>(value);
            i_target->set(i_target->get() + i_value->get());
            break;
        }
        default:
            throw std::runtime_error("Unknown binary op");
        }
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
            skip_next();
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
            skip_next();
        break;
    }
    case NodeType::Dictionary:
    {
        uint32_t size;
        m_data >> size;
        for(uint32_t i = 0; i < size; ++i)
        {
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
            skip_next();
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
    
    for(auto &e: list)
    {
        //FIXME support other types
        auto s = memory_manager().create_string(e);
        l->append(s);
    }

    set_value(name, l);
}

}


