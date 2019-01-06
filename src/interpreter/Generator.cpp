#include <cowlang/Dictionary.h>
#include <cowlang/Object.h>
#include <cowlang/Scope.h>

namespace cow
{

TuplePtr MemoryManager::create_tuple() { return make_value<Tuple>(*this); }

StringValPtr MemoryManager::create_string(const std::string &str)
{
    return make_value<StringVal>(*this, str);
}

IntValPtr MemoryManager::create_integer(const int32_t value)
{
    return make_value<IntVal>(*this, value);
}

BoolValPtr MemoryManager::create_boolean(const bool value)
{
    return make_value<BoolVal>(*this, value);
}

DictionaryPtr MemoryManager::create_dictionary() { return make_value<Dictionary>(*this); }

FloatValPtr MemoryManager::create_float(const double &value)
{
    return make_value<FloatVal>(*this, value);
}

ValuePtr MemoryManager::create_none() { return std::shared_ptr<Value>{ nullptr }; }

ListPtr MemoryManager::create_list() { return make_value<List>(*this); }


} // namespace cow
