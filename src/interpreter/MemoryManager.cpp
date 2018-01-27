#include <cowlang/Object.h>
#include <cowlang/execution_limits.h>
#include <cassert>

namespace cow
{

MemoryManager::MemoryManager()
    : m_buffer(), m_buffer_pos(0)
{
    m_buffer = new uint8_t[PAGE_SIZE];
}

MemoryManager::~MemoryManager()
{
    delete m_buffer;
}

void* DummyMemoryManager::malloc(size_t size)
{
    return ::malloc(size);
}

void DummyMemoryManager::free(void *ptr)
{
    ::free(ptr);
}

void* MemoryManager::malloc(size_t size)
{
    auto last_pos = reinterpret_cast<intptr_t>(&m_buffer[0]);

    for(auto &[start, a]: m_allocs)
    {
        assert(last_pos <= start);

        auto s = static_cast<size_t>(start - last_pos);

        if(s >= size)
        {
            auto ptr = reinterpret_cast<uint8_t*>(last_pos);
            auto idx = reinterpret_cast<intptr_t>(ptr);

            m_allocs.emplace(idx, AllocInfo{size});
            return ptr;
        }
        else
        {
            last_pos = start + a.size;
        }
    }

    if(m_buffer_pos+size >= PAGE_SIZE)
    {
        //FIXME allocate new pages if allowed
        throw execution_limit_exception("Out of memory!");
    }

    auto ptr = reinterpret_cast<uint8_t*>(last_pos);
    auto idx = reinterpret_cast<intptr_t>(ptr);

    m_buffer_pos += size;

    m_allocs.emplace(idx, AllocInfo{size});
    return ptr;
}

void MemoryManager::free(void *ptr)
{
    auto idx = reinterpret_cast<intptr_t>(ptr);
    auto it = m_allocs.find(idx);

    if(it == m_allocs.end())
    {
        throw std::runtime_error("Memory error!");
    }
    else
    {
        m_allocs.erase(it);
    }
}

}
