#include <stdbitstream.h>
#include "json/json.h"

namespace json
{

Writer::Writer(bitstream &result)
    : m_result_ptr(nullptr), m_result(result)
{
}

Writer::Writer()
    : m_result_ptr(new bitstream), m_result(*m_result_ptr)
{}

Writer::~Writer()
{
    delete m_result_ptr;
}

json::Document Writer::make_document()
{
    if(m_result.empty())
    {
        m_result << ObjectType::Null;
    }

    uint8_t *data;
    uint32_t len;
    m_result.detach(data, len);

    return json::Document(data, len, DocumentMode::ReadWrite);
}

void Writer::start_array(const std::string &key)
{
    handle_key(key);

    m_result << ObjectType::Array;

    uint32_t start_pos = m_result.pos();
    uint32_t byte_size = 0, size = 0;
    m_result << byte_size << size;

    m_starts.push(start_pos);
    m_sizes.push(size);
    m_mode.push(IN_ARRAY);
}

void Writer::end_array()
{
    uint32_t end_pos = m_result.pos();
    uint32_t start_pos = m_starts.top();
    m_result.move_to(start_pos);
    uint32_t byte_size = end_pos - (start_pos + sizeof(uint32_t));
    uint32_t size = m_sizes.top();

    m_result << byte_size << size;
    m_result.move_to(end_pos);

    if(m_mode.empty() || m_mode.top() != IN_ARRAY)
    {
        throw json_error("Writer::end_array failed: Invalid state");
    }

    m_mode.pop();
    m_starts.pop();
    m_sizes.pop();

    check_end();
}

void Writer::start_map(const std::string &key)
{
    handle_key(key);

    m_result << ObjectType::Map;

    uint32_t start = m_result.pos();
    uint32_t byte_size = 0, size = 0;
    m_result << byte_size;
    m_result << size;

    m_mode.push(IN_MAP);
    m_sizes.push(size);
    m_starts.push(start);
}

void Writer::end_map()
{
    uint32_t end_pos = m_result.pos();
    uint32_t start_pos = m_starts.top();
    uint32_t size = m_sizes.top();
    m_result.move_to(start_pos);
    uint32_t byte_size = (end_pos - (start_pos + sizeof(uint32_t)));

    m_result << byte_size << size;

    m_result.move_to(end_pos);

    if(m_mode.empty() || m_mode.top() != IN_MAP)
    {
        throw json_error("Writer::end_map failed: Invalid state");
    }

    m_sizes.pop();
    m_starts.pop();
    m_mode.pop();

    check_end();
}

void Writer::check_end()
{
    if(m_mode.empty())
    {
        m_mode.push(DONE);
    }
}

void Writer::write_raw_data(const std::string &key, const uint8_t *data, uint32_t size)
{
    handle_key(key);
    m_result.write_raw_data(data, size);
    check_end();
}

void Writer::write_binary(const std::string &key, const bitstream &value)
{
    handle_key(key);
    m_result << ObjectType::Binary << static_cast<uint32_t>(value.size());
    m_result.write_raw_data(value.data(), value.size());
}

#ifdef USE_GEO
void Writer::write_vector2(const std::string &key, const geo::vector2d &vec)
{
    handle_key(key);
    m_result << ObjectType::Vector2 << vec.X << vec.Y;
    check_end();
}
#endif

void Writer::write_float(const std::string &key, const double &value)
{
    handle_key(key);
    m_result << ObjectType::Float << value;
    check_end();
}

void Writer::write_integer(const std::string &key, const integer_t &value)
{
    handle_key(key);
    m_result << ObjectType::Integer << value;
    check_end();
}

void Writer::write_boolean(const std::string &key, const bool value)
{
    handle_key(key);

    if(value)
    {
        m_result << ObjectType::True;
    }
    else
    {
        m_result << ObjectType::False;
    }

    check_end();
}

void Writer::write_null(const std::string &key)
{
    handle_key(key);
    m_result << ObjectType::Null;
    check_end();
}

void Writer::write_datetime(const std::string &key, const tm &value)
{
    handle_key(key);
    m_result << ObjectType::Datetime;
    m_result << value;
    check_end();
}

void Writer::write_string(const std::string &key, const std::string &value)
{
    handle_key(key);
    m_result << ObjectType::String << value;
    check_end();
}

void Writer::handle_key(const std::string &key)
{
    if(key.empty())
    {
        if(m_mode.empty())
        {
            return;
        }
        else if(m_mode.top() != IN_ARRAY)
        {
            throw json_error("Empty key only valid for initial object or in arrays!");
        }
    }

    if(m_mode.empty())
    {
        throw json_error("Writer::handle_key failed: Initial key needs to be empty string.");
    }

    if(m_mode.top() == DONE)
    {
        throw json_error("Cannot write more. Already done");
    }

    auto size = m_sizes.top();
    m_sizes.pop();
    m_sizes.push(size+1);

    if(m_mode.top() == IN_MAP)
    {
        m_result << key;
    }
}

}
