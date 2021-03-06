#pragma once

#include "RangeIterator.h"
#include <cowlang/cow.h>

#include "args.h"
#include <cowlang/Value.h>
#include <cowlang/unpack.h>
#include <iostream>
#include <stdio.h>

extern void print_program_output(const std::string &str);

namespace cow
{

enum class BuiltinType
{
    Range,
    MakeInt,
    MakeString,
    Min,
    Max,
    Print,
    Length,
};

class Builtin : public Callable
{
public:
    Builtin(MemoryManager &mem, BuiltinType type) : Callable(mem), m_type(type) {}

    ValuePtr duplicate(MemoryManager &mem) override
    {
        return ValuePtr(new(mem) Builtin(mem, m_type));
    }

    ValueType type() const override { return ValueType::Builtin; }

    ValuePtr call(const std::vector<ValuePtr> &args, Scope &scope, uint32_t &, uint32_t &) override
    {

        // No nullpointers please
        for(size_t i = 0; i < args.size(); ++i)
            ASSERT_GENERIC(args[i]);

        if(m_type == BuiltinType::Range)
        {
            int min, max, step;

            if(args.size() == 1)
            {
                step = 1;
                min = 0;
                max = unpack_integer(args[0]);
            }
            else if(args.size() == 2)
            {
                step = 1;
                min = unpack_integer(args[0]);
                max = unpack_integer(args[1]);
            }
            else if(args.size() == 3)
            {
                min = unpack_integer(args[0]);
                max = unpack_integer(args[1]);
                step = unpack_integer(args[2]);
            }
            else
            {
                throw std::runtime_error("Invalid number of arguments");
            }

            return ValuePtr(new(memory_manager()) RangeIterator(memory_manager(), min, max, step));
        }
        else if(m_type == BuiltinType::MakeString)
        {
            check_num_args(args, 1);

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
        else if(m_type == BuiltinType::Min || m_type == BuiltinType::Max)
        {
            check_num_args(args, 2);

            auto arg1 = args[0];
            auto arg2 = args[1];

            check_is_integer(arg1);
            check_is_integer(arg2);

            if(arg1 == nullptr || arg1->type() != ValueType::Integer)
            {
            }

            if(arg2 == nullptr || arg2->type() != ValueType::Integer)
            {
                throw std::runtime_error("First argument of min/max");
            }

            int result;
            auto i1 = value_cast<IntVal>(arg1)->get();
            auto i2 = value_cast<IntVal>(arg2)->get();

            if(m_type == BuiltinType::Max)
            {
                result = std::max(i1, i2);
            }
            else
            {
                result = std::min(i1, i2);
            }

            return memory_manager().create_integer(result);
        }
        else if(m_type == BuiltinType::MakeInt)
        {
            check_num_args(args, 1);

            auto arg = args[0];
            if(arg->type() == ValueType::Integer)
            {
                return arg;
            }
            else if(arg->type() == ValueType::String)
            {
                std::string s = value_cast<StringVal>(arg)->get();
                char *endptr = nullptr;
                return wrap_value(new(memory_manager())
                                  IntVal(memory_manager(), strtol(s.c_str(), &endptr, 10)));
            }
            else
            {
                throw std::runtime_error("Can not convert to integer");
            }
        }
        else if(m_type == BuiltinType::Length)
        {
            check_num_args(args, 1);

            auto arg = args[0];
            return wrap_value(new(memory_manager()) IntVal(memory_manager(), arg->size()));
        }
        else if(m_type == BuiltinType::Print)
        {
            check_num_minargs(args, 1);
            for(size_t i = 0; i < args.size(); ++i)
            {
                std::string prefix = "";
                if(i > 0)
                    prefix = " ";

                auto arg = args[i];
                if(relaxed_check_is_string(arg))
                    print_program_output(prefix + value_cast<StringVal>(arg)->get());
                else
                {
                    if(!arg)
                    {
                        print_program_output(
                        prefix + value_cast<StringVal>(memory_manager().create_string("None"))->get());
                    }
                    else
                    {
                        print_program_output(
                        prefix + value_cast<StringVal>(memory_manager().create_string(arg->str()))->get());
                    }
                }
            }
            print_program_output("\n");
        }
        else
        {
            throw std::runtime_error("Unknown builtin type");
        }

        return nullptr;
    }

private:
    const BuiltinType m_type;
};

} // namespace cow
