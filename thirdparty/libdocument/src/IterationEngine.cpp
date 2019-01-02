#include "json.h"
#include "Iterator.h"
#include "json/json_error.h"

#include "defines.h"
#include <string>
using std::to_string;

namespace json
{

IterationEngine::IterationEngine(const bitstream &data, json::Iterator &iterator_)
    : iterator(iterator_)
{
    if(data.empty())
    {
        throw json_error("Cannot iterate: not a valid JSON object!");
    }

    view.assign(data.data(), data.size(), true);
}

void IterationEngine::run()
{
    parse_next("");
}

void IterationEngine::parse_next(const std::string &key)
{
    ObjectType type;
    view >> type;

    switch(type)
    {
    case ObjectType::String:
    {
        std::string str;
        view >> str;
        iterator.handle_string(key, str);
        break;
    }
    case ObjectType::Integer:
    {
        integer_t i;
        view >> i;
        iterator.handle_integer(key, i);
        break;
    }
    case ObjectType::Float:
    {
        double val;
        view >> val;
        iterator.handle_float(key, val);
        break;
    }
    case ObjectType::Datetime:
    {
        tm val;
        view >> val;
        iterator.handle_datetime(key, val);
        break;
    }
    case ObjectType::Binary:
    {
        uint32_t size = 0;
        uint8_t *data = nullptr;
        view >> size;
        view.read_raw_data(&data, size);
        iterator.handle_binary(key, data, size);
        break;
    }
    case ObjectType::Map:
    {
        handle_map(key);
        break;
    }
    case ObjectType::Array:
    {
        handle_array(key);
        break;
    }
    case ObjectType::True:
    {
        iterator.handle_boolean(key, true);
        break;
    }
    case ObjectType::False:
    {
        iterator.handle_boolean(key, false);
        break;
    }
    case ObjectType::Null:
    {
        iterator.handle_null(key);
        break;
    }
    default:
    {
        throw json_error("Document Iteration failed: unknown object type");
    }
    }
}

void IterationEngine::handle_map(const std::string &key)
{
    iterator.handle_map_start(key);

    uint32_t byte_size = 0;
    view >> byte_size;

    uint32_t size = 0;
    view >> size;

    for(uint32_t i = 0; i < size; ++i)
    {
        std::string key;
        view >> key;
        parse_next(key);
    }

    iterator.handle_map_end();
}

void IterationEngine::handle_array(const std::string &key)
{
    iterator.handle_array_start(key);

    uint32_t byte_size = 0, size = 0;
    view >> byte_size >> size;

    for(uint32_t i = 0; i < size; ++i)
    {
        parse_next(to_string(i));
    }

    iterator.handle_array_end();
}

}
