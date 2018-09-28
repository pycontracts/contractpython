#pragma once

#include "Value.h"

namespace cow
{

/// Helper functions to quickly convert from cow to std

inline bool unpack_bool(ValuePtr val)
{
    auto bval = value_cast<BoolVal>(val);
    return bval->get();
}

inline int32_t unpack_integer(ValuePtr val)
{
    if(val == nullptr || val->type() != ValueType::Integer)
    {
        throw std::runtime_error("value is not an integer");
    }

    auto ival = value_cast<IntVal>(val);
    return ival->get();
}

inline float unpack_float(ValuePtr val)
{
    if(val->type() == ValueType::Float)
    {
        auto fval = value_cast<FloatVal>(val);
        return fval->get();
    }
    else if(val->type() == ValueType::Integer)
    {
        return static_cast<float>(unpack_integer(val));
    }
    else
        throw cow::value_exception("cannot extract float!");
}

inline std::string unpack_string(ValuePtr val)
{
    auto sval = value_cast<StringVal>(val);
    return sval->get();
}
}
