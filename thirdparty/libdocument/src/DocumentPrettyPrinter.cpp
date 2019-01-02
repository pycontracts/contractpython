#include "json.h"
#include "Iterator.h"

#include "defines.h"
using std::to_string;

namespace json
{

DocumentPrettyPrinter::DocumentPrettyPrinter(int indent):
    m_indent(indent), m_current_indent(0), m_is_first(true)
{}

void DocumentPrettyPrinter::handle_string(const std::string &key, const std::string &value)
{
    print_indent();
    print_key(key);
    m_res += '\"';
    m_res += value;
    m_res += '\"';
}

void DocumentPrettyPrinter::handle_integer(const std::string &key, const json::integer_t value)
{
    print_indent();
    print_key(key);
    m_res += to_string(value);
}

void DocumentPrettyPrinter::handle_float(const std::string &key, const json::float_t value)
{
    print_indent();
    print_key(key);
    m_res += to_string(value);
}

void DocumentPrettyPrinter::handle_map_start(const std::string &key)
{
    print_indent();
    print_key(key);
    m_res += "{\n";
    indent();
    m_is_first = true;
    m_is_array.push(false);
}

void DocumentPrettyPrinter::handle_boolean(const std::string &key, const bool value)
{
    print_indent();
    print_key(key);
    m_res += (value ? "true" : "false");
}

void DocumentPrettyPrinter::handle_null(const std::string &key)
{
    print_indent();
    print_key(key);
    m_res += "null";
}

void DocumentPrettyPrinter::handle_map_end()
{
    unindent();
    m_res += '\n';
    m_is_first = true;
    print_indent();
    m_res += '}';
    m_is_array.pop();
}

void DocumentPrettyPrinter::handle_array_start(const std::string &key)
{
    print_indent();
    print_key(key);
    m_res += "[\n";
    indent();
    m_is_first = true;
    m_is_array.push(true);
}

void DocumentPrettyPrinter::handle_array_end()
{
    unindent();
    m_res += '\n';
    m_is_first = true;
    print_indent();
    m_res += ']';
    m_is_array.pop();
}

void DocumentPrettyPrinter::handle_binary(const std::string &key, const uint8_t *data, uint32_t size)
{
    (void)data;

    print_indent();
    print_key(key);
    m_res += "<binary data, length ";
    m_res += to_string(size);
    m_res += '>';
}

void DocumentPrettyPrinter::handle_datetime(const std::string &key, const tm& value)
{
    print_indent();
    print_key(key);
    m_res += to_string(value.tm_year, 4) + "-" + to_string(value.tm_mon, 2) + "-" + to_string(value.tm_mday, 2);
    m_res += " " + to_string(value.tm_hour, 2) + ":" + to_string(value.tm_min, 2) + ":" + to_string(value.tm_sec, 2);
}

void DocumentPrettyPrinter::print_key(const std::string &key)
{
    if(key.empty())
    {
        return;
    }

    if(m_is_array.top())
    {
        return;
    }
   
    m_res += '\"';
    m_res += key;
    m_res += "\": ";
}

void DocumentPrettyPrinter::print_indent()
{
    if (m_is_first)
    {
        m_is_first = false;
    }
    else
    {
        m_res += ",\n";
    }

    for (int i = 0; i < m_current_indent; ++i)
    {
        m_res += ' ';
    }
}

void DocumentPrettyPrinter::indent()
{
    m_current_indent += m_indent;
}

void DocumentPrettyPrinter::unindent()
{
    m_current_indent -= m_indent;
}

}
