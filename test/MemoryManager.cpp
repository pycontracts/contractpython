#include <cowlang/Object.h>
#include <gtest/gtest.h>

namespace cow
{

class MemoryManagerTest : public testing::Test
{
};

TEST(MemoryManager, simple_realloc)
{
    DefaultMemoryManager mem;
    const size_t size = 420;

    auto ptr = mem.malloc(size);
    mem.free(ptr);
    ptr = mem.malloc(size);

    auto buf = reinterpret_cast<uint8_t*>(ptr);

    memset(buf, 0, size);
    buf[5] = 1;

    uint8_t sum = 0;

    for(uint32_t i = 0; i < size; ++i)
    {
        sum += buf[i];
    }

    EXPECT_EQ(1, sum);
    
    mem.free(ptr);
}

TEST(MemoryManager, malloc_free)
{
    DefaultMemoryManager mem;
    const size_t size = 420;

    auto ptr = mem.malloc(size);
    auto buf = reinterpret_cast<uint8_t*>(ptr);

    memset(buf, 0, size);
    buf[5] = 1;

    uint8_t sum = 0;

    for(uint32_t i = 0; i < size; ++i)
    {
        sum += buf[i];
    }

    EXPECT_EQ(1, sum);
    
    mem.free(ptr);
}

TEST(MemoryManager, malloc_few)
{
    DefaultMemoryManager mem;
    constexpr size_t SIZE = 50;
    constexpr size_t NUM = 4;

    void* ptrs[NUM];

    for(uint32_t i = 0; i < NUM; ++i)
    {
        ptrs[i] = mem.malloc(SIZE);
        memset(ptrs[i], static_cast<uint8_t>(i), SIZE);
    }

    for(uint32_t i = 0; i < NUM; ++i)
    {
        auto val = reinterpret_cast<uint8_t*>(ptrs[i])[2]; 
        EXPECT_EQ(i, val);

        mem.free(ptrs[i]);
    }
}

TEST(MemoryManager, malloc_many)
{
    DefaultMemoryManager mem;
    constexpr size_t SIZE = 50;
    constexpr size_t NUM = 1000;

    void* ptrs[NUM];

    for(uint32_t i = 0; i < NUM; ++i)
    {
        ptrs[i] = mem.malloc(SIZE);
        memset(ptrs[i], static_cast<uint8_t>(i % 255), SIZE);
    }

    for(uint32_t i = 0; i < NUM; ++i)
    {
        auto val = reinterpret_cast<uint8_t*>(ptrs[i])[2]; 
        EXPECT_EQ(i % 255, val);
        mem.free(ptrs[i]);
    }
}

TEST(MemoryManager, realloc)
{
    DefaultMemoryManager mem;
    constexpr size_t SIZE = 50;
    constexpr size_t NUM = 10;

    void* ptrs[NUM];

    for(uint32_t i = 0; i < NUM; ++i)
    {
        ptrs[i] = mem.malloc(SIZE);
    }

    for(uint32_t i = 0; i < NUM; ++i)
    {
        if(i % 2 == 0)
        {
            continue;
        }

        mem.free(ptrs[i]);
    }

    auto ptr = mem.malloc(SIZE);
    auto buf = reinterpret_cast<uint8_t*>(ptr);
    buf[12] = 'F';

    EXPECT_EQ('F', buf[12]);
}

}
