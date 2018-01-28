#include <cowlang/Object.h>
#include <cowlang/execution_limits.h>
#include <cassert>

namespace cow
{

MemoryManager::MemoryManager()
    : m_buffer_pos(0)
{
    auto buffer = new uint8_t[PAGE_SIZE];
    m_buffers.push_back(buffer);
}

MemoryManager::~MemoryManager()
{
    for(auto buffer: m_buffers)
    {
        delete buffer;
    }
}

void* DummyMemoryManager::malloc(size_t size)
{
    return ::malloc(size);
}

void DummyMemoryManager::free(void *ptr)
{
    ::free(ptr);
}

void* MemoryManager::assign_alloc(size_t page_no, size_t poffset, size_t size)
{
    auto ptr = reinterpret_cast<uint8_t*>(&m_buffers[page_no][poffset]);
    auto idx = reinterpret_cast<intptr_t>(ptr);

    m_allocs.emplace(idx, AllocInfo{page_no, size});
    return ptr;
}

void* MemoryManager::malloc(size_t size)
{
    if(size >= PAGE_SIZE)
    {
        throw std::runtime_error("cannot allocate more than page size");
    }

    auto last_pos = 0;

    uint32_t pos = 0;
    for(auto buffer: m_buffers)
    {
        for(auto &[ptr, a]: m_allocs)
        {
            if(pos != a.page)
            {
                continue;
            }

            auto start = ptr - reinterpret_cast<size_t>(buffer);
            auto s = static_cast<size_t>(start - last_pos);

            if(s >= size)
            {
                return assign_alloc(pos, last_pos, size);
            }
            else
            {
                last_pos = start + a.size;
            }
        }

        pos += 1;
    }

    auto buffer_size = m_buffers.size() * PAGE_SIZE;

    if(m_buffer_pos+size >= buffer_size)
    {
        auto buffer = new uint8_t[PAGE_SIZE];
        m_buffers.push_back(buffer);
        m_buffer_pos = buffer_size;
        
        //TODO support execution limits
        //  throw execution_limit_exception("Out of memory!");
    }

    m_buffer_pos += size;
    return assign_alloc(m_buffers.size() - 1, m_buffer_pos - ((m_buffers.size() - 1) * PAGE_SIZE), size);
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
