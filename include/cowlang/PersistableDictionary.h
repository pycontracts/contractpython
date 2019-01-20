#pragma once

#include "Callable.h"
#include "Iterator.h"
#include "Scope.h"
#include "Value.h"
#include "unpack.h"

namespace cow
{

class PersistableDictionary;
typedef std::shared_ptr<PersistableDictionary> PersistableDictionaryPtr;

class PersistableDictionary : public Value
{
public:
    PersistableDictionary(MemoryManager &mem) : Value(mem) {}
    static void operator delete(void *ptr)
    {
        // this object is not on memorymanager's heap
    }
    ValuePtr get(const std::string &key);
    void insert(const std::string &key, ValuePtr value);
    void apply(const std::string &key, ValuePtr value, BinaryOpType op);

    void remove(const std::string &key);
    bool has(const std::string &key);
    void clear();
    ValueType type() const override { return ValueType::PersistableDictionary; }
    ValuePtr duplicate(MemoryManager &mem) override
    {
        throw std::runtime_error("Persistent dictionaries cannot be copied.");
    }


    std::map<std::string, std::string> m_elements_string;
    std::map<std::string, int64_t> m_elements_int;
    std::map<std::string, double> m_elements_double;
    std::map<std::string, bool> m_elements_bool;
};

} // namespace cow
