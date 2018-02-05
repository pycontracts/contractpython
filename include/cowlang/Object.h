#pragma once

#include <map>
#include <vector>
#include <memory>

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

typedef std::shared_ptr<Value> ValuePtr;
typedef std::shared_ptr<IntVal> IntValPtr;
typedef std::shared_ptr<Dictionary> DictionaryPtr;
typedef std::shared_ptr<List> ListPtr;
typedef std::shared_ptr<StringVal> StringValPtr;
typedef std::shared_ptr<Tuple> TuplePtr;
typedef std::shared_ptr<BoolVal> BoolValPtr;
typedef std::shared_ptr<FloatVal> FloatValPtr;

/**
 * Interface for memory management
 */
class MemoryManager
{
public:
    MemoryManager() = default;
    MemoryManager(MemoryManager &other) = delete;

    virtual ~MemoryManager() {}

    virtual void* malloc(size_t sz) = 0;
    virtual void free(void* ptr) = 0;

    IntValPtr create_integer(const int32_t value);
    DictionaryPtr create_dictionary();
    StringValPtr create_string(const std::string &str);
    TuplePtr create_tuple();
    ValuePtr create_from_document(const json::Document &doc);
    FloatValPtr create_float(const double &f);
    BoolValPtr create_boolean(const bool value);
    ListPtr create_list();
    ValuePtr create_none();
};

class DefaultMemoryManager : public MemoryManager
{
public:
   DefaultMemoryManager();
   ~DefaultMemoryManager();

   static constexpr size_t PAGE_SIZE = 1024*1024;

   void* malloc(size_t sz) override;
   void free(void* ptr) override;

private:
    void* assign_alloc(size_t page_no, size_t poffset, size_t size);

    std::vector<uint8_t*> m_buffers;
    size_t m_buffer_pos;

    struct AllocInfo
    {
        size_t page;
        size_t size;
    };

    void resize(size_t new_size);

    //Needs to be ordered
    std::map<intptr_t, AllocInfo> m_allocs;

};

/**
 * @brief Memory manager that will *not* manage it's own memory
 */
class DummyMemoryManager : public MemoryManager
{
public:
    DummyMemoryManager() = default;

    void* malloc(size_t sz) override;
    void free(void* ptr) override;
};

class Object
{
public:
    virtual ~Object() {}

    static void* operator new(std::size_t sz, MemoryManager &mem_mgr)
    {
        auto ptr = mem_mgr.malloc(sz);
        return ptr;
    }

    static void operator delete(void *ptr)
    {
        auto obj = reinterpret_cast<Object*>(ptr);
        obj->m_mem.free(ptr);
    }

    MemoryManager& memory_manager()
    {
        return m_mem;
    }

protected:
    Object(MemoryManager &mem)
        : m_mem(mem)
    {}

    MemoryManager &m_mem;
};

}
