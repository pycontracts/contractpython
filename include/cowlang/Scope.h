#pragma once

#include <unordered_map>

#include "List.h"
#include "Tuple.h"
#include "Value.h"
#include "Dictionary.h"

namespace cow
{

class Scope : public Object
{
public:
    static constexpr const char* BUILTIN_STR_NONE = "None";
    static constexpr const char* BUILTIN_STR_RANGE = "range";
    static constexpr const char* BUILTIN_STR_MAKE_INT = "int";
    static constexpr const char* BUILTIN_STR_MAKE_STR = "str";
    static constexpr const char* BUILTIN_STR_PRINT = "print";
    static constexpr const char* BUILTIN_STR_LENGTH = "len";
    static constexpr const char* BUILTIN_STR_MAX = "max";
    static constexpr const char* BUILTIN_STR_MIN = "min";

    Scope(MemoryManager &mem) : Object(mem), m_parent(nullptr) {}
    Scope(MemoryManager &mem, Scope &parent) : Object(mem), m_parent(&parent) {}

    ValuePtr get_value(const std::string &id);
    void set_value(const std::string &name, ValuePtr value);
    bool has_value(const std::string &name) const;
    void terminate();
    bool is_terminated() const;

private:
    Scope *m_parent;
    bool m_terminated = false;

    std::unordered_map<std::string, ValuePtr> m_values;
};

}
