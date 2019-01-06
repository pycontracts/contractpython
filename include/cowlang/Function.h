#pragma once

#include <functional>

#include "Callable.h"
#include "Value.h"

namespace cow
{

/**
 * @brief Wrapper for a lambda function
 */
class Function : public Callable
{
public:
    Function(MemoryManager &mem, std::function<ValuePtr(const std::vector<ValuePtr> &)> func)
    : Callable(mem), m_func(func)
    {
    }

    ValuePtr duplicate(MemoryManager &mem) override
    {
        return wrap_value(new(mem) Function(mem, m_func));
    }

    ValuePtr call(const std::vector<ValuePtr> &args, Scope &scope) override { return m_func(args); }

    ValueType type() const override { return ValueType::Function; }

private:
    const std::function<ValuePtr(const std::vector<ValuePtr>)> m_func;
};

} // namespace cow
