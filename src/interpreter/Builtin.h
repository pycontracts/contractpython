#pragma once

#include "RangeIterator.h"

#include <cowlang/Value.h>
#include <cowlang/unpack.h>

#ifdef IS_ENCLAVE
extern void print_program_output(const std::string &str);
#else

#include <glog/logging.h>

inline void print_program_output(const std::string &str)
{
    LOG(INFO) << str;
}
#endif

namespace cow
{

enum class BuiltinType
{
    Range,
    MakeInt,
    MakeString,
    Print
};

class Builtin : public Callable
{
public:
    Builtin(MemoryManager &mem, BuiltinType type)
        : Callable(mem), m_type(type)
    {}

    ValuePtr duplicate(MemoryManager &mem) override
    {
        return ValuePtr(new (mem) Builtin(mem, m_type));
    }

    ValueType type() const override
    {
        return ValueType::Builtin;
    }

    ValuePtr call(const std::vector<ValuePtr> &args) override
    {
        if(m_type == BuiltinType::Range)
        {
            //TODO implemeent the rest...
            if(args.size() != 1)
                throw std::runtime_error("Invalid number of arguments");

            auto arg = args[0];

            if(arg == nullptr || arg->type() != ValueType::Integer)
                throw std::runtime_error("invalid argument type");

            return ValuePtr(new (memory_manager()) RangeIterator(memory_manager(), 0, value_cast<IntVal>(arg)->get(), 1));
        }
        else if(m_type == BuiltinType::MakeString)
        {
            if(args.size() != 1)
                throw std::runtime_error("Invalid number of arguments");

            auto arg = args[0];
            if(!arg)
            {
                return memory_manager().create_string("None");
            }   
            else
            {
                return memory_manager().create_string(arg->str());
            }
        }
        else if(m_type == BuiltinType::MakeInt)
        {
            if(args.size() != 1)
                throw std::runtime_error("Invalid number of arguments");

            auto arg = args[0];
            if(arg->type() == ValueType::Integer)
            {
                return arg;
            }
            else if(arg->type() == ValueType::String)
            {
                std::string s = value_cast<StringVal>(arg)->get();
                char *endptr = nullptr;
                return wrap_value(new (memory_manager()) IntVal(memory_manager(), strtol(s.c_str(), &endptr, 10)));
            }
            else
                throw std::runtime_error("Can't conver to integer");
        }
        else if(m_type == BuiltinType::Print)
        {
            if(args.size() != 1)
                throw std::runtime_error("Invalid number of arguments");

            auto arg = args[0];

            if(arg->type() != ValueType::String)
                throw std::runtime_error("Argument not a string");

            print_program_output(("Program says: ") + value_cast<StringVal>(arg)->get());
        }
        else
            throw std::runtime_error("Unknown builtin type");

        return nullptr;
    }

private:
    const BuiltinType m_type;
};

}
