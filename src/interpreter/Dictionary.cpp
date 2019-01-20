#include <cowlang/Dictionary.h>
#include <cowlang/Tuple.h>

namespace cow
{

DictItemIterator::DictItemIterator(MemoryManager &mem, Dictionary &dict)
: Generator(mem), m_dict(dict), m_it(dict.elements().begin())
{
}

ValuePtr DictItemIterator::next()
{
    if(m_it == m_dict.elements().end())
        throw stop_iteration_exception();

    auto key = memory_manager().create_string(m_it->first);
    auto t = memory_manager().create_tuple();
    t->append(key);
    t->append(m_it->second);
    m_it++;
    return t;
}

ValuePtr DictItemIterator::duplicate(MemoryManager &mem)
{
    return wrap_value(new(mem) DictItemIterator(mem, m_dict));
}

DictKeyIterator::DictKeyIterator(MemoryManager &mem, Dictionary &dict)
: Generator(mem), m_dict(dict), m_it(dict.elements().begin())
{
}

DictItemsPtr Dictionary::items() { return make_value<DictItems>(memory_manager(), *this); }

ValuePtr DictKeyIterator::next()
{
    if(m_it == m_dict.elements().end())
        throw stop_iteration_exception();

    // FIXME implement tuples
    auto &elem = m_it->second;
    m_it++;
    return elem;
}

ValuePtr DictKeyIterator::duplicate(MemoryManager &mem)
{
    return wrap_value(new(mem) DictKeyIterator(mem, m_dict));
}

ValuePtr DictItems::duplicate(MemoryManager &mem)
{
    return wrap_value(new(mem) DictItems(mem, m_dict));
}

IteratorPtr DictItems::iterate()
{
    return wrap_value(new(memory_manager()) DictItemIterator(memory_manager(), m_dict));
}

const std::map<std::string, ValuePtr> &Dictionary::elements() const { return m_elements; }

std::map<std::string, ValuePtr> &Dictionary::elements() { return m_elements; }

uint32_t Dictionary::size() const { return m_elements.size(); }

IteratorPtr Dictionary::iterate()
{
    return wrap_value(new(memory_manager()) DictKeyIterator(memory_manager(), *this));
}

ValuePtr Dictionary::get(const std::string &key)
{
    auto it = m_elements.find(key);
    if(it == m_elements.end())
        return nullptr;

    return it->second;
}

void Dictionary::insert(const std::string &key, ValuePtr value) { m_elements[key] = value; }

ValueType Dictionary::type() const { return ValueType::Dictionary; }

ValuePtr Dictionary::duplicate(MemoryManager &mem)
{
    auto d = wrap_value(new(mem) Dictionary(mem));

    for(auto &it : m_elements)
    {
        d->insert(it.first, it.second);
    }

    return d;
}


void Dictionary::apply(const std::string &key, ValuePtr value, BinaryOpType op)
{

    ValuePtr target = nullptr;
    auto it = m_elements.find(key);
    if(it != m_elements.end())
        target = it->second;

    if(!value || (target != nullptr && target->type() != ValueType::Integer) || value->type() != ValueType::Integer)
    {
        throw std::runtime_error("Values need to be numerics");
    }

    switch(op)
    {
    case BinaryOpType::Add:
    {
        if(target == nullptr)
        {
            auto i_value = value_cast<IntVal>(value);
            m_elements[key] = i_value;
        }
        else
        {
            auto i_target = value_cast<IntVal>(target);
            auto i_value = value_cast<IntVal>(value);
            i_target->set(i_target->get() + i_value->get());
        }
        break;
    }
    case BinaryOpType::Sub:
    {
        if(target == nullptr)
        {
            auto i_value = value_cast<IntVal>(value);
            i_value->set(-1 * i_value->get());
            m_elements[key] = i_value;
        }
        else
        {
            auto i_target = value_cast<IntVal>(target);
            auto i_value = value_cast<IntVal>(value);
            i_target->set(i_target->get() - i_value->get());
        }
        break;
    }
    case BinaryOpType::Mult:
    {
        if(target == nullptr)
        {
            auto zero = wrap_value(new(memory_manager()) IntVal(memory_manager(), 0));
            m_elements[key] = zero;
        }
        else
        {
            auto i_target = value_cast<IntVal>(target);
            auto i_value = value_cast<IntVal>(value);
            i_target->set(i_target->get() + i_value->get());
        }
        break;
    }
    default:
        throw std::runtime_error("Unknown binary op");
    }
}

} // namespace cow
