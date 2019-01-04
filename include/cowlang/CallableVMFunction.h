#ifndef CALLABLE_VM
#define CALLABLE_VM

#include <functional>

#include "Value.h"
#include "Callable.h"

namespace cow
{

/**
 * @brief Wrapper for inner-VM function
 */
class CallableVMFunction : public Callable
{
public:
    CallableVMFunction(MemoryManager &mem, bitstream& begin_jump, std::vector<std::string>& args, std::vector<ValuePtr>& defaults, Interpreter& _i)
        : Callable(mem), m_begin_jump(std::move(begin_jump)), m_args(args), m_defaults(defaults), i(_i)
    {}

    ValuePtr duplicate(MemoryManager &mem) override
    {
        return wrap_value(new (mem) CallableVMFunction(mem, m_begin_jump, m_args, m_defaults, i));
    }

    ValuePtr call(const std::vector<ValuePtr>& args) override
    {
        ValuePtr returnval = nullptr;
        // call with own context
        Scope body_scope(memory_manager(), i.get_scope());
        body_scope.require_global();
        Interpreter pyint(m_begin_jump, memory_manager(), i); // borrow the scope of the parent interpreter

        // set default values to the args first
        uint32_t minimum_arguments = 0;
        uint32_t maximum_arguments = m_args.size();
        for(size_t i=0; i < maximum_arguments; ++i){
            if(m_defaults[i] != nullptr)
                pyint.set_value(m_args[i], m_defaults[i]);
            else {
                minimum_arguments ++; // this argument must be provided as it has no def. value
            }
        }

        if(args.size()<minimum_arguments || args.size()>maximum_arguments){
            throw std::runtime_error("You did not proved the correct number of arguments to the function");
        }else{
            for(size_t i=0; i < args.size(); ++i){
                pyint.set_value(m_args[i], args[i]);
            }
        }

        returnval = pyint.execute_in_scope(body_scope);

        return returnval;
    }

    ValueType type() const override { return ValueType::Function; }

private:
    Interpreter& i;
    bitstream m_begin_jump;
    std::vector<std::string> m_args;
    std::vector<ValuePtr> m_defaults;
};

}

#endif
