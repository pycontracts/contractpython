#pragma once

#include <cowlang/Iterator.h>

namespace cow
{

class RangeIterator : public Generator
{
public:
    RangeIterator(MemoryManager &mem, int32_t start, int32_t end, int32_t step_size)
        : Generator(mem),
        m_start(start), m_end(end), m_step_size(step_size)
    {
        m_pos = m_start;
    }

    ValuePtr duplicate(MemoryManager &mem) override
    {
        return wrap_value(new (mem) RangeIterator(mem, m_start, m_end, m_step_size));
    }

    ValuePtr next() override
    {
        if(m_pos >= m_end)
            throw stop_iteration_exception();

        auto res = wrap_value(new (memory_manager()) IntVal(memory_manager(), m_pos));
        m_pos += m_step_size;

        return res;
    }

private:
    int32_t m_pos;
    const int32_t m_start, m_end, m_step_size;
};

}

