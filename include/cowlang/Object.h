#pragma once

#include <map>
#include <memory>
#include <vector>

namespace json
{
class Document;
}


namespace cow
{

class Value;
class IntVal;
class Dictionary;
class StringVal;
class Tuple;
class List;
class BoolVal;
class FloatVal;

using ValuePtr = std::shared_ptr<Value>;
using IntValPtr = std::shared_ptr<IntVal>;
using DictionaryPtr = std::shared_ptr<Dictionary>;
using ListPtr = std::shared_ptr<List>;
using StringValPtr = std::shared_ptr<StringVal>;
using TuplePtr = std::shared_ptr<Tuple>;
using BoolValPtr = std::shared_ptr<BoolVal>;
using FloatValPtr = std::shared_ptr<FloatVal>;

// Default value for maximum heap memory size -> 512MB!
// If a program tries to allocate more, it will throw an OutOfMemoryError
extern size_t DEFAULT_MAXIMUM_HEAP_PAGES;

/**
 * Interface for memory management
 */
class MemoryManager
{
public:
    MemoryManager() = default;
    MemoryManager(MemoryManager &other) = delete;

    virtual ~MemoryManager() {}

    virtual void *malloc(size_t size) = 0;
    virtual void free(void *ptr) = 0;
    virtual const uint32_t get_max_mem() = 0;
    virtual const uint32_t get_mem() = 0;

    IntValPtr create_integer(const int32_t value);
    DictionaryPtr create_dictionary();
    StringValPtr create_string(const std::string &str);
    TuplePtr create_tuple();
    ValuePtr create_from_document(const json::Document &doc);
    FloatValPtr create_float(const double &value);
    BoolValPtr create_boolean(const bool value);
    ListPtr create_list();
    ValuePtr create_none();
};

class DefaultMemoryManager : public MemoryManager
{
public:
    DefaultMemoryManager();
    ~DefaultMemoryManager();

    static constexpr size_t PAGE_SIZE = 1024 * 1024;

    void *malloc(size_t size) override;
    void free(void *ptr) override;

    const uint32_t get_max_mem() override;
    const uint32_t get_mem() override;

private:
    void *assign_alloc(size_t page_no, size_t poffset, size_t size);

    std::vector<uint8_t *> m_buffers;
    size_t m_buffer_pos;

    struct AllocInfo
    {
        size_t page;
        size_t size;
    };

    void resize(size_t new_size);

    // Needs to be ordered
    std::map<intptr_t, AllocInfo> m_allocs;
};

/**
 * @brief Memory manager that will *not* manage it's own memory
 */
class DummyMemoryManager : public MemoryManager
{
public:
    DummyMemoryManager() = default;

    void *malloc(size_t sz) override;
    void free(void *ptr) override;
    const uint32_t get_max_mem() override;
    const uint32_t get_mem() override;
};

class Object
{
public:
    virtual ~Object() {}

    static void *operator new(std::size_t sz, MemoryManager &mem_mgr)
    {
        auto ptr = mem_mgr.malloc(sz);
        return ptr;
    }

    static void operator delete(void *ptr)
    {
        auto obj = reinterpret_cast<Object *>(ptr);
        obj->m_mem.free(ptr);
    }

    MemoryManager &memory_manager() { return m_mem; }


protected:
    Object(MemoryManager &mem) : m_mem(mem) {}

    MemoryManager &m_mem;
};

} // namespace cow
