#include <cowlang/Dictionary.h>
#include <cowlang/List.h>
#include <cowlang/Scope.h>

#include "Builtin.h"

namespace cow
{

void Scope::set_value(const std::string &name, ValuePtr value)
{
    if(m_parent && m_parent->has_value(name))
    {
        m_parent->set_value(name, value);
        return;
    }

    // FIXME actually update references...
    auto it = m_values.find(name);
    if(it != m_values.end())
    {
        m_values.erase(it);
    }

    m_values.emplace(name, std::move(value));
}

bool Scope::has_value(const std::string &name) const
{
    if(m_parent && m_parent->has_value(name))
    {
        return true;
    }

    return m_values.find(name) != m_values.end();
}

ValuePtr Scope::get_value(const std::string &name)
{
    Value *val = nullptr;

    if(name == BUILTIN_STR_NONE)
    {
        return std::shared_ptr<Value>{ nullptr };
    }

    if(name == BUILTIN_STR_RANGE)
    {
        val = new(memory_manager()) Builtin(memory_manager(), BuiltinType::Range);
    }
    else if(name == BUILTIN_STR_MAKE_INT)
    {
        val = new(memory_manager()) Builtin(memory_manager(), BuiltinType::MakeInt);
    }
    else if(name == BUILTIN_STR_MAKE_STR)
    {
        val = new(memory_manager()) Builtin(memory_manager(), BuiltinType::MakeString);
    }
    else if(name == BUILTIN_STR_PRINT)
    {
        val = new(memory_manager()) Builtin(memory_manager(), BuiltinType::Print);
    }
    else if(name == BUILTIN_STR_LENGTH)
    {
        val = new(memory_manager()) Builtin(memory_manager(), BuiltinType::Length);
    }
    else if(name == BUILTIN_STR_MIN)
    {
        val = new(memory_manager()) Builtin(memory_manager(), BuiltinType::Min);
    }
    else if(name == BUILTIN_STR_MAX)
    {
        val = new(memory_manager()) Builtin(memory_manager(), BuiltinType::Max);
    }

    if(val)
    {
        return std::shared_ptr<Value>(val);
    }

    auto it = m_values.find(name);
    if(it == m_values.end())
    {
        if(m_parent)
        {
            return m_parent->get_value(name);
        }
        
        throw std::runtime_error("No such value: " + name);
    }

    return it->second;
}

void Scope::terminate() { m_terminated = true; }

bool Scope::is_terminated() const { return m_terminated; }


} // namespace cow
