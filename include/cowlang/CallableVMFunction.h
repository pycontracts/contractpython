#ifndef CALLABLE_VM
#define CALLABLE_VM

#include "Callable.h"
#include "Scope.h"
#include "Value.h"
#include <cowlang/Interpreter.h>
#include <functional>
#include <iostream>
namespace cow
{

/**
 * @brief Wrapper for inner-VM function
 */
class CallableVMFunction : public Callable
{
public:
    CallableVMFunction(MemoryManager &mem,
                       bitstream &begin_jump,
                       std::vector<std::string> &args,
                       std::vector<ValuePtr> &defaults)
    : Callable(mem), m_begin_jump(std::move(begin_jump)), m_args(args), m_defaults(defaults)
    {
    }

    ValuePtr duplicate(MemoryManager &mem) override
    {
        return wrap_value(new(mem) CallableVMFunction(mem, m_begin_jump, m_args, m_defaults));
    }

    ValuePtr call(const std::vector<ValuePtr> &args, Scope &scope, uint32_t &current_num, uint32_t &current_max) override
    {
        ValuePtr returnval = nullptr;
        // call with own context
        Scope body_scope(memory_manager(), scope);
        body_scope.require_global();
        Interpreter pyint(m_begin_jump, memory_manager()); // borrow the scope of the parent interpreter
        pyint.set_execution_step_limit(current_max);
        pyint.set_num_execution_steps(current_num);

        // set default values to the args first
        uint32_t minimum_arguments = 0;
        uint32_t maximum_arguments = m_args.size();

        if(args.size() > maximum_arguments)
        {
            throw std::runtime_error(
            "You did not provide the correct number of arguments to the function");
        }

        for(size_t i = 0; i < maximum_arguments; ++i)
        {
            if(m_defaults[i] != nullptr)
                body_scope.set_value(m_args[i], m_defaults[i]);
            else
            {
                minimum_arguments++; // this argument must be provided as it has no def. value
            }
        }

        if(args.size() < minimum_arguments)
        {
            throw std::runtime_error(
            "You did not provide the correct number of arguments to the function");
        }

        for(size_t i = 0; i < args.size(); ++i)
        {
            body_scope.set_value(m_args[i], args[i]);
        }


        try
        {
            returnval = pyint.execute_in_scope(body_scope);
        }
        catch(...)
        {
            current_num = pyint.num_execution_steps();
            throw;
        }
        current_num = pyint.num_execution_steps();
        return returnval;
    }

    ValueType type() const override { return ValueType::Function; }

private:
    bitstream m_begin_jump;
    std::vector<std::string> m_args;
    std::vector<ValuePtr> m_defaults;
};

} // namespace cow

#endif
