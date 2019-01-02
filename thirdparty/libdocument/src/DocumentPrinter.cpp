#include "json.h"
#include "Iterator.h"

#include "defines.h"
using std::to_string;

namespace json
{

void Printer::handle_key(const std::string &key)
{
    const auto& m = mode.top();

    if(key.empty())
    {
        if(!mode.empty())
        {
            throw json_error("invalid state: key stack is empty but mode isn't");
        }
        return;
    }
    else if(m == FIRST_IN_MAP)
    {
        result += '"' + key + "\":";
        mode.pop();
        mode.push(IN_MAP);
    }
    else if(m == IN_MAP)
    {
        result += ",\"" + key + "\":";
    }
    else if(m == FIRST_IN_ARRAY)
    {
        mode.pop();
        mode.push(IN_ARRAY);
    }
    else if(m == IN_ARRAY)
    {
        result += ",";
    }
    else
    {
        throw json_error("Unknown parse mode");
    }
}

void Printer::handle_string(const std::string &key, const std::string &value)
{
    handle_key(key);
    result += '"' + value + '"';
}

void Printer::handle_integer(const std::string &key, const integer_t value)
{
    handle_key(key);
    result += to_string(value);
}

void Printer::handle_float(const std::string &key, const json::float_t value)
{
    handle_key(key);
    result += to_string(value);
}

void Printer::handle_map_start(const std::string &key)
{
    handle_key(key);
    mode.push(FIRST_IN_MAP);
    result += "{";
}

void Printer::handle_binary(const std::string &key, const uint8_t *data, uint32_t len)
{
    handle_key(key);
    result += "b'";

    for(uint32_t i = 0; i < len; ++i)
    {
        uint8_t val = data[i];
        uint8_t up = (0xF0) & val << 4;
        uint8_t down = (0x0F) & val;

        result += to_hex(up);
        result += to_hex(down);
    }

    result += '\'';
}

void Printer::handle_boolean(const std::string &key, const bool value)
{
    handle_key(key);

    if(value)
    {
        result += keyword(TRUE);
    }
    else
    {
        result += keyword(FALSE);
    }
}

void Printer::handle_null(const std::string &key)
{
    handle_key(key);
    result += keyword(NIL);
}

void Printer::handle_datetime(const std::string &key, const tm& value)
{
    handle_key(key);

    result += "d\"";
    result += to_string(value.tm_year, 4) + "-" + to_string(value.tm_mon, 2) + "-" + to_string(value.tm_mday, 2);
    result += " " + to_string(value.tm_hour, 2) + ":" + to_string(value.tm_min, 2) + ":" + to_string(value.tm_sec, 2);
    result += '"';
}

void Printer::handle_map_end()
{
    mode.pop();
    result += "}";
}

void Printer::handle_array_start(const std::string &key)
{
    handle_key(key);
    result += "[";
    mode.push(FIRST_IN_ARRAY);
}

void Printer::handle_array_end()
{
    result += "]";
    mode.pop();
}



}
