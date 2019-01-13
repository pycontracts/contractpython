#ifndef CALLABLE_VM
#define CALLABLE_VM

#include <functional>

#include "Callable.h"
#include "Scope.h"
#include "Value.h"

namespace cow
{

/**
 * @brief Wrapper for C function proxy
 */
class CallableCFunction : public Callable
{
public:
    CallableCFunction(MemoryManager &mem, std::vector<std::string> &args, std::function<ValuePtr(Scope &s)> _func)
    : Callable(mem), m_args(args), func(_func)
    {
    }

    ValuePtr duplicate(MemoryManager &mem) override
    {
        return wrap_value(new(mem) CallableCFunction(mem, m_args, func));
    }

    ValuePtr call(const std::vector<ValuePtr> &args, Scope &scope, uint32_t &current_num, uint32_t &current_max) override
    {
        ValuePtr returnval = nullptr;
        // call with own context
        Scope body_scope(memory_manager(), scope);
        body_scope.require_global();

        // set default values to the args first
        uint32_t minimum_arguments = m_args.size();
        uint32_t maximum_arguments = m_args.size();

        if(args.size() < minimum_arguments || args.size() > maximum_arguments)
        {
            throw std::runtime_error(
            "You did not provide the correct number of arguments to the function");
        }
        else
        {
            for(size_t i = 0; i < args.size(); ++i)
            {
                body_scope.set_value(m_args[i], args[i]);
            }
        }

        returnval = func(body_scope);

        return returnval;
    }

    ValueType type() const override { return ValueType::Function; }

private:
    std::vector<std::string> m_args;
    std::function<ValuePtr(Scope &s)> func;
};

} // namespace cow

#endif
