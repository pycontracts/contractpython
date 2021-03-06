#pragma once

#include "InterpreterTypes.h"
#include "Iterator.h"
namespace cow
{

class List;
typedef std::shared_ptr<List> ListPtr;

class ListIterator : public Generator
{
public:
    ListIterator(MemoryManager &mem, List &list);

    ValuePtr next() override;

    ValuePtr duplicate(MemoryManager &mem) override;

private:
    List &m_list;
    uint32_t m_pos;
};

class List : public IterateableValue
{
public:
    List(MemoryManager &mem) : IterateableValue(mem) {}

    IteratorPtr iterate() override;

    ValuePtr duplicate(MemoryManager &mem) override;

    ValuePtr get(int64_t index);

    void set(int64_t index, ValuePtr val);
    void apply(int64_t key, ValuePtr value, BinaryOpType op);


    ValuePtr get_member(const std::string &name) override;

    uint32_t size() const override;

    std::string str() const override;

    bool contains(const Value &value) const;

    ValueType type() const override;

    void append(ValuePtr val);

    const std::vector<ValuePtr> &elements() const;

private:
    std::vector<ValuePtr> m_elements;
};

} // namespace cow
