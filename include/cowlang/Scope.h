#pragma once

#include <set>
#include <unordered_map>

#include "List.h"
#include "Tuple.h"
#include "Value.h"

namespace cow
{

class Scope : public Object
{
public:
    static constexpr const char *BUILTIN_STR_NONE = "None";
    static constexpr const char *BUILTIN_STR_RANGE = "range";
    static constexpr const char *BUILTIN_STR_MAKE_INT = "int";
    static constexpr const char *BUILTIN_STR_MAKE_STR = "str";
    static constexpr const char *BUILTIN_STR_PRINT = "print";
    static constexpr const char *BUILTIN_STR_LENGTH = "len";
    static constexpr const char *BUILTIN_STR_MAX = "max";
    static constexpr const char *BUILTIN_STR_MIN = "min";
    static constexpr const char *BUILTIN_STR_CLEAR = "clear";
    static constexpr const char *BUILTIN_STR_CLEARLIMITS = "clearlimits";

    Scope(MemoryManager &mem) : Object(mem), m_parent(nullptr), depth(0) {}
    Scope(MemoryManager &mem, Scope &parent)
    : Object(mem), m_parent(&parent), depth(parent.depth + 1)
    {
        if(depth >= 128)
        {
            throw std::runtime_error("You have exceeded the maximum recursion depth of 128");
        }
    }

    ValuePtr get_value(const std::string &id);
    void set_value(const std::string &name, ValuePtr value);
    bool has_value(const std::string &name) const;
    void set_global_tag(const std::string &name);
    void terminate();
    bool is_terminated() const;
    int get_depth() { return depth; }
    void require_global() { m_require_global = true; };

private:
    Scope *m_parent;
    bool m_terminated = false;
    bool m_require_global = false;
    int depth;

    std::unordered_map<std::string, ValuePtr> m_values;
    std::set<std::string> m_global_tags;
};

} // namespace cow
