#include <cassert>
#include <cowlang/Object.h>
#include <cowlang/execution_limits.h>

#include <iostream>

namespace cow
{

DefaultMemoryManager::DefaultMemoryManager() : m_buffer_pos(0)
{
    auto buffer = new uint8_t[PAGE_SIZE];
    m_buffers.push_back(buffer);
}

DefaultMemoryManager::~DefaultMemoryManager()
{
    for(auto buffer : m_buffers)
    {
        delete buffer;
    }
}

void *DummyMemoryManager::malloc(size_t size) { return ::malloc(size); }

void DummyMemoryManager::free(void *ptr) { ::free(ptr); }

void *DefaultMemoryManager::assign_alloc(size_t page_no, size_t poffset, size_t size)
{
    if(poffset + size > PAGE_SIZE)
    {
        throw std::runtime_error("Invalid offset");
    }

    auto ptr = reinterpret_cast<uint8_t *>(&m_buffers[page_no][poffset]);
    auto idx = reinterpret_cast<intptr_t>(ptr);

    m_allocs.emplace(idx, AllocInfo{ page_no, size });
    return ptr;
}

void *DefaultMemoryManager::malloc(size_t size)
{
    if(size >= PAGE_SIZE)
    {
        throw std::runtime_error("cannot allocate more than page size");
    }

    uint32_t buf_pos = 0;
    for(auto buffer : m_buffers)
    {
        auto last_pos = 0;

        for(auto &[ptr, a] : m_allocs)
        {
            if(buf_pos != a.page)
            {
                continue;
            }

            auto start = ptr - reinterpret_cast<size_t>(buffer);
            auto s = static_cast<size_t>(start - last_pos);

            if(s >= size)
            {
                return assign_alloc(buf_pos, last_pos, size);
            }

            last_pos = start + a.size;
        }

        buf_pos += 1;
    }

    auto buffer_size = m_buffers.size() * PAGE_SIZE;

    if(m_buffer_pos + size >= buffer_size)
    {
        auto buffer = new uint8_t[PAGE_SIZE];
        m_buffers.push_back(buffer);
        m_buffer_pos = buffer_size;

        // TODO support execution limits
        //  throw execution_limit_exception("Out of memory!");
    }

    auto new_pos = m_buffer_pos;
    m_buffer_pos += size;

    auto buffer = m_buffers.size() - 1;
    auto offset = new_pos - (buffer * PAGE_SIZE);

    return assign_alloc(buffer, offset, size);
}

void DefaultMemoryManager::free(void *ptr)
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

} // namespace cow
