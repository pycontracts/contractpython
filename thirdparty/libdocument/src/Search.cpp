#include "Search.h"

namespace json
{

Search::Search(const Document &document, const std::string &path)
    : m_document(document), m_target_path(path)
{
    m_view.assign(m_document.data().data(), m_document.data().size(), true);
}

void Search::parse_next()
{
    auto current = path_string(m_current_path);
    bool on_path = false;
    bool on_target = false;

    auto len = std::min(current.size(), m_target_path.size());

    if(len == 0)
    {
        if(m_target_path.size() == len)
        {
            on_path = on_target = true;
        }
        else
        {
            on_path = true;
        }
    }
    else if(m_target_path.compare(0, len, current) == 0)
    {
        if(m_target_path.size() == len)
        {
            on_path = on_target = true;
        }
        else if(m_target_path[len] == '.')
        {
            on_path = true;
        }
    }

    std::string key;
    if(!m_current_path.empty())
    {
        key = m_current_path.back();
    }

    uint32_t start = m_view.pos();

    ObjectType type;
    m_view >> type;

    if(!on_path)
    {
        skip_next(type, m_view);
        return;
    }

    if(on_target)
    {
        skip_next(type, m_view);

        uint32_t end = m_view.pos();

        m_result.assign(&m_view.data()[start], end-start, true);
        m_success = true;
        return;
    }

    switch(type)
    {
    case ObjectType::String:
    {
        std::string str;
        m_view >> str;
        break;
    }
    case ObjectType::Integer:
    {
        json::integer_t i;
        m_view >> i;
        break;
    }
    case ObjectType::Float:
    {
        json::float_t d;
        m_view >> d;
        break;
    }
    case ObjectType::Map:
    {
        parse_map();
        break;
    }
    case ObjectType::Array:
    {
        parse_array();
        break;
    }
    case ObjectType::Binary:
    {
        uint32_t len = 0;
        m_view >> len;
        m_view.move_by(len);
        break;
    }
    case ObjectType::True:
    case ObjectType::False:
    case ObjectType::Null:
        break;
    default:
        throw json_error("Document search failed: Unknown object type");
    }
}

void Search::parse_map()
{
    uint32_t byte_size = 0;
    m_view >> byte_size;

    uint32_t size = 0;
    m_view >> size;

    std::string key = m_current_path.empty() ? "": m_current_path.back();

    for(uint32_t i = 0; i < size; ++i)
    {
        std::string key;
        m_view >> key;

        m_current_path.push_back(key);
        parse_next();
        m_current_path.pop_back();
    }
}

void Search::parse_array()
{
    uint32_t byte_size = 0;
    m_view >> byte_size;

    uint32_t size = 0;
    m_view >> size;

    std::string key = m_current_path.empty() ? "": m_current_path.back();

    for(uint32_t i = 0; i < size; ++i)
    {
        m_current_path.push_back(to_string(i));
        parse_next();
        m_current_path.pop_back();
    }
}


}
