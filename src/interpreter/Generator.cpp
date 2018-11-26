#include <cowlang/Object.h>
#include <cowlang/Scope.h>

namespace cow
{

TuplePtr MemoryManager::create_tuple() { return make_value<Tuple>(*this); }

StringValPtr MemoryManager::create_string(const std::string &str)
{
    return make_value<StringVal>(*this, str);
}

IntValPtr MemoryManager::create_integer(const int32_t value)
{
    return make_value<IntVal>(*this, value);
}

BoolValPtr MemoryManager::create_boolean(const bool value)
{
    return make_value<BoolVal>(*this, value);
}

DictionaryPtr MemoryManager::create_dictionary()
{
    return make_value<Dictionary>(*this);
}

FloatValPtr MemoryManager::create_float(const double &value)
{
    return make_value<FloatVal>(*this, value);
}

ValuePtr MemoryManager::create_none() { return std::shared_ptr<Value>{ nullptr }; }

ListPtr MemoryManager::create_list() { return make_value<List>(*this); }

class DocConverter : public json::Iterator
{
public:
    explicit DocConverter(MemoryManager &mem) : m_mem(mem) {}

    cow::ValuePtr get_result()
    {
        // Not a valid document?
        if(parse_stack.size() != 1)
        {
            throw std::runtime_error("Json converter in an invalid child");
        }

        return parse_stack.top();
    }

    void handle_datetime(const std::string &key, const tm &value) override
    {
        (void)key;
        (void)value;
        throw std::runtime_error("Datetime not supported");
    }

    void handle_string(const std::string &key, const std::string &str) override
    {
        auto val = m_mem.create_string(str);
        add_value(key, val);
    }

    void handle_integer(const std::string &key, int64_t i) override
    {
        auto val = m_mem.create_integer(i);
        add_value(key, val);
    }

    void handle_float(const std::string &key, const double value) override
    {
        auto val = m_mem.create_float(value);
        add_value(key, val);
    }

    void handle_boolean(const std::string &key, const bool value) override
    {
        auto val = m_mem.create_boolean(value);
        add_value(key, val);
    }

    void add_value(const std::string &key, const ValuePtr &value)
    {
        if(key.empty())
        {
            parse_stack.push(value);
        }
        else
        {
            append_child(key, value);
        }
    }

    void handle_null(const std::string &key) override
    {
        auto val = m_mem.create_none();
        add_value(key, val);
    }

    void handle_map_start(const std::string &key) override
    {
        auto dict = m_mem.create_dictionary();

        if(!key.empty())
        {
            append_child(key, dict);
        }

        parse_stack.push(dict);
    }

    void handle_map_end() override
    {
        if(parse_stack.size() > 1)
        {
            parse_stack.pop();
        }
    }

    void handle_array_start(const std::string &key) override
    {
        auto list = m_mem.create_list();
        if(!key.empty())
        {
            append_child(key, list);
        }

        parse_stack.push(list);
    }

    void handle_array_end() override
    {
        if(parse_stack.size() > 1)
        {
            parse_stack.pop();
        }
    }

    void handle_binary(const std::string &key, const uint8_t *data, uint32_t len) override
    {
        // FIXME
        (void)data;
        (void)len;

        auto val = m_mem.create_integer(0);
        add_value(key, val);
    }

private:
    void append_child(const std::string key, const ValuePtr &obj)
    {
        if(parse_stack.empty())
        {
            throw std::runtime_error("cannot append child at this point!");
        }

        auto &top = parse_stack.top();

        switch(top->type())
        {
        case ValueType::Dictionary:
        {
            auto d = value_cast<Dictionary>(top);
            d->insert(key, obj);
            break;
        }
        case ValueType::List:
        {
            auto l = value_cast<List>(top);
            l->append(obj);
            break;
        }
        default:
            throw std::runtime_error("Failed to append child");
        }
    }

    MemoryManager &m_mem;
    std::stack<ValuePtr> parse_stack;
};

ValuePtr MemoryManager::create_from_document(const json::Document &doc)
{
    if(!doc.valid() || doc.empty())
    {
        return nullptr;
    }

    DocConverter converter(*this);
    doc.iterate(converter);
    return converter.get_result();
}

} // namespace cow
