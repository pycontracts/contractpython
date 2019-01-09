#include <cowlang/PersistableDictionary.h>

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
