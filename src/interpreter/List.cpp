#include <cowlang/Function.h>
#include <cowlang/Interpreter.h>
#include <cowlang/List.h>
namespace cow
{

IteratorPtr List::iterate()
{
    return wrap_value(new(memory_manager()) ListIterator(memory_manager(), *this));
}

ValuePtr List::duplicate(MemoryManager &mem)
{
    auto d = wrap_value(new(mem) List(mem));

    for(auto elem : m_elements)
    {
        d->append(elem);
    }

    return d;
}

ValuePtr List::get_member(const std::string &name)
{
    auto &mem = memory_manager();

    if(name == "append")
    {
        return wrap_value(new(mem) Function(mem, [&](const std::vector<ValuePtr> &args) -> ValuePtr {
            if(args.size() != 1)
            {
                throw std::runtime_error("Invalid number of arguments");
            }

            this->append(args[0]);
            return nullptr;
        }));
    }
    else
    {
        throw language_exception("No such member List::" + name);
    }
}

ValuePtr List::get(int64_t index)
{
    if(index >= size() || index <= -(int32_t)size())
    {
        throw std::runtime_error("List index out of range");
    }
    if(index < 0)
        index = size() + index;
    return m_elements[index];
}

void List::set(int64_t index, ValuePtr val)
{
    if(index >= size() || index <= -size())
    {
        throw std::runtime_error("List index out of range");
    }
    if(index < 0)
        index = size() + index;

    m_elements[index] = val;
}

uint32_t List::size() const { return m_elements.size(); }

const std::vector<ValuePtr> &List::elements() const { return m_elements; }

std::string List::str() const
{
    std::string result = "[";
    bool first = true;

    for(auto elem : m_elements)
    {
        if(first)
        {
            first = false;
        }
        else
        {
            result += ", ";
        }

        if(elem == nullptr)
        {
            result += "None";
        }
        else
        {

            if(elem->type() == ValueType::String)
            {
                result += '\'' + elem->str() + '\'';
            }
            else
            {
                result += elem->str();
            }
        }
    }

    return result + ']';
}

bool List::contains(const Value &value) const
{
    for(auto elem : m_elements)
    {
        ASSERT_GENERIC(elem);
        if(*elem == value)
            return true;
    }

    return false;
}

ValueType List::type() const { return ValueType::List; }

void List::append(ValuePtr val) { m_elements.push_back(val); }

ListIterator::ListIterator(MemoryManager &mem, List &list) : Generator(mem), m_list(list), m_pos(0)
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
    return ValuePtr(new(mem) ListIterator(mem, m_list));
}

} // namespace cow
