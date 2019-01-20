#include <cowlang/PersistableDictionary.h>
#include <iostream>
ValuePtr PersistableDictionary::get(const std::string &key)
{
    {
        auto it = m_elements_string.find(key);
        if(it != m_elements_string.end())
            return wrap_value(new(memory_manager()) StringVal(memory_manager(), it->second));
    }
    {
        auto it = m_elements_int.find(key);
        if(it != m_elements_int.end())
            return wrap_value(new(memory_manager()) IntVal(memory_manager(), it->second));
    }
    {
        auto it = m_elements_double.find(key);
        if(it != m_elements_double.end())
            return wrap_value(new(memory_manager()) FloatVal(memory_manager(), it->second));
    }
    {
        auto it = m_elements_bool.find(key);
        if(it != m_elements_bool.end())
            return wrap_value(new(memory_manager()) BoolVal(memory_manager(), it->second));
    }
    return nullptr;
}

void PersistableDictionary::apply(const std::string &key, ValuePtr value, BinaryOpType op)
{

    int64_t target = 0;
    auto itx = m_elements_bool.find(key);
    auto ity = m_elements_double.find(key);
    auto itz = m_elements_string.find(key);
    if(itx != m_elements_bool.end())
        throw std::runtime_error("Values need to be numerics");
    if(ity != m_elements_double.end())
        throw std::runtime_error("Values need to be numerics");
    if(itz != m_elements_string.end())
        throw std::runtime_error("Values need to be numerics");

    auto it = m_elements_int.find(key);
    if(it != m_elements_int.end())
        target = it->second;

    if(!value || value->type() != ValueType::Integer)
    {
        throw std::runtime_error("Values need to be numerics");
    }

    switch(op)
    {
    case BinaryOpType::Add:
    {

        auto i_value = value_cast<IntVal>(value);
        m_elements_int[key] = i_value->get() + target;

        break;
    }
    case BinaryOpType::Sub:
    {

        auto i_value = value_cast<IntVal>(value);
        m_elements_int[key] = target - i_value->get();

        break;
    }
    case BinaryOpType::Mult:
    {

        auto i_value = value_cast<IntVal>(value);
        m_elements_int[key] = target * i_value->get();

        break;
    }
    default:
        throw std::runtime_error("Unknown binary op");
    }
}
void PersistableDictionary::insert(const std::string &key, ValuePtr value)
{
    if(value->type() == ValueType::String)
    {
        std::string _value = unpack_string(value);
        remove(key);
        m_elements_string[key] = _value;
    }
    else if(value->type() == ValueType::Integer)
    {
        int64_t _value = unpack_integer(value);
        remove(key);
        m_elements_int[key] = _value;
    }
    else if(value->type() == ValueType::Bool)
    {
        bool _value = unpack_bool(value);
        remove(key);
        m_elements_bool[key] = _value;
    }
    else if(value->type() == ValueType::Float)
    {
        double _value = unpack_float(value);
        remove(key);
        m_elements_double[key] = _value;
    }
    else
    {
        throw std::runtime_error(
        "Persistent directories can only store string, int, float and boolean.");
    }
}

void PersistableDictionary::clear()
{
    m_elements_string.clear();
    m_elements_int.clear();
    m_elements_bool.clear();
    m_elements_double.clear();
}

void PersistableDictionary::remove(const std::string &key)
{
    {
        auto it = m_elements_string.find(key);
        if(it != m_elements_string.end())
            m_elements_string.erase(it);
    }
    {
        auto it = m_elements_int.find(key);
        if(it != m_elements_int.end())
            m_elements_int.erase(it);
    }
    {
        auto it = m_elements_int.find(key);
        if(it != m_elements_int.end())
            m_elements_int.erase(it);
    }
    {
        auto it = m_elements_double.find(key);
        if(it != m_elements_double.end())
            m_elements_double.erase(it);
    }
}

bool PersistableDictionary::has(const std::string &key)
{
    {
        auto it = m_elements_string.find(key);
        if(it != m_elements_string.end())
            return true;
    }
    {
        auto it = m_elements_int.find(key);
        if(it != m_elements_int.end())
            return true;
    }
    {
        auto it = m_elements_int.find(key);
        if(it != m_elements_int.end())
            return true;
    }
    {
        auto it = m_elements_double.find(key);
        if(it != m_elements_double.end())
            return true;
    }
    return false;
}
