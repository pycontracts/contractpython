#pragma once

#include <cowlang/Value.h>

namespace cow
{

inline void check_num_args(const std::vector<ValuePtr> &args, size_t expected)
{
    if(args.size() != expected)
    {
        throw std::runtime_error("Invalid number of arguments");
    }
}

inline void check_num_minargs(const std::vector<ValuePtr> &args, size_t expected)
{
    if(args.size() < expected)
    {
        throw std::runtime_error("Invalid number of arguments");
    }
}

inline void check_is_integer(const ValuePtr arg)
{
    if(!arg || arg->type() != ValueType::Integer)
    {
        throw std::runtime_error("Argument is not an integer");
    }
}

inline void check_is_string(const ValuePtr arg)
{
    if(!arg || arg->type() != ValueType::String)
    {
        throw std::runtime_error("Argument is not a string");
    }
}

inline bool relaxed_check_is_string(const ValuePtr arg)
{
    if(!arg || arg->type() != ValueType::String)
    {
        return false;
    }
    return true;
}

} // namespace cow
