#pragma once

#include <functional>
#include <memory>

#include "Value.h"
#include "Callable.h"

namespace cow
{

class Module;
typedef std::shared_ptr<Module> ModulePtr;
    
class Module : public Value
{
public:
    Module(MemoryManager &mem)
        : Value(mem)
    {}

    virtual ~Module() {}

    ValueType type() const override { return ValueType::Module; }

    virtual ValuePtr get_member(const std::string& name) = 0;

    ValuePtr duplicate(MemoryManager&) override
    {
        return nullptr; //not supported
    }
};

/**
 * @brief Wrapper for a lambda function
 */
class Function : public Callable
{
public:
    Function(MemoryManager &mem, std::function<ValuePtr(const std::vector<ValuePtr>&)> func)
        : Callable(mem), m_func(func)
    {}

    ValuePtr duplicate(MemoryManager &mem) override
    {
        return wrap_value(new (mem) Function(mem, m_func));
    }

    ValuePtr call(const std::vector<ValuePtr>& args) override
    {
        return m_func(args);
    }

    ValueType type() const override { return ValueType::Function; }

private:
    const std::function<ValuePtr (const std::vector<ValuePtr>)> m_func;
};

}
