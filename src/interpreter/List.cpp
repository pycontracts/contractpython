#include <cowlang/List.h>

namespace cow
{

IteratorPtr List::iterate()
{
    return wrap_value(new (memory_manager()) ListIterator(memory_manager(), *this));
}

ValuePtr List::duplicate(MemoryManager &mem)
{
    auto d = wrap_value(new (mem) List(mem));

    for(auto elem: m_elements)
    {
        d->append(elem);
    }

    return d;
}

ValuePtr List::get(uint32_t index)
{
    if(index >= size())
        throw std::runtime_error("List index out of range");

    return m_elements[index];
}

uint32_t List::size() const
{
    return m_elements.size();
}

const std::vector<ValuePtr>& List::elements() const
{
    return m_elements;
}

bool List::contains(const Value &value) const
{
    for(auto elem: m_elements)
    {
        if(*elem == value)
            return true;
    }

    return false;
}

ValueType List::type() const
{
    return ValueType::List;
}

void List::append(ValuePtr val)
{
    m_elements.push_back(val);
}

ListIterator::ListIterator(MemoryManager &mem, List &list)
    : Generator(mem), m_list(list), m_pos(0)
{
}

ValuePtr ListIterator::next() 
{
    if(m_pos >= m_list.size())
        throw stop_iteration_exception();

    auto res = m_list.get(m_pos);
    m_pos += 1;

    return res;
}

ValuePtr ListIterator::duplicate(MemoryManager &mem)
{
    return ValuePtr(new (mem) ListIterator(mem, m_list));
}

}
