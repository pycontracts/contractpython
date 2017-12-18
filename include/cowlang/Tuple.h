#pragma once

#include "Value.h"

namespace cow
{

class Tuple : public Value
{
public:
    Tuple(MemoryManager &mem)
        : Value(mem), m_content()
    {
    }

    Tuple(MemoryManager &mem, const Tuple &other)
        : Value(mem), m_content(other.m_content)
    {}

    ValueType type() const override
    {
        return ValueType::Tuple;
    }

    ValuePtr duplicate(MemoryManager &mem) override
    {
        return ValuePtr(new (mem) Tuple(mem, *this));
    }

    void append(ValuePtr val)
    {
        m_content.push_back(val);
    }

    uint32_t size() const
    {
        return m_content.size();
    }

    ValuePtr get(uint32_t pos)
    {
        return m_content[pos];
    }

private:
    std::vector<ValuePtr> m_content;
};

typedef std::shared_ptr<Tuple> TuplePtr;

}
