#include <cow/Object.h>

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
/*    if(m_buffer_pos+size >= m_buffer.size())
    {
        m_buffer.resize(m_buffer_pos+size);
    }*/

    auto ptr = &m_buffer[m_buffer_pos];
    auto idx = reinterpret_cast<intptr_t>(ptr);

    m_buffer_pos += size;

    m_allocs.emplace(idx, AllocInfo{size});
    return ptr;
}

void MemoryManager::free(void *ptr)
{   
    auto idx = reinterpret_cast<intptr_t>(ptr);
    auto it = m_allocs.find(idx);
    m_allocs.erase(it);

    //FIXME: actually reuse memory...
}


}
