#pragma once

#include <stdint.h>
#include <string>
#include <map>
#include <stack>
#include <cctype>
#include <vector>
#include <time.h>

#include <ostream>

namespace json
{

class Document;

enum class ObjectType : uint8_t
{
    Map,
    Array,
    String,
    Datetime,
    Integer,
    Float,
    True,
    False,
    Binary,
    Vector2,
    Null
};

enum class DocumentMode
{
    ReadOnly,
    ReadWrite,
    Copy
};


inline bool is_valid_key(const std::string &str)
{
    if(str.size() == 0)
        return false;

    for(auto c: str)
    {
        if(!isalnum(c) && !(c == '_'))
            return false;
    }

    return true;
}

inline bool is_keyword(const std::string &str)
{
    return str == "+";
}

typedef int64_t integer_t;
typedef double float_t;

}
