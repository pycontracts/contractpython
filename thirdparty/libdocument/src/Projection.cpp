#include "Projection.h"

namespace json
{

Projection::Projection(const Document &document, const std::vector<std::string> &paths, bool write_path)
    : m_document(document), m_write_path(write_path), m_found_count(0)
{
    m_view.assign(m_document.data().data(), m_document.data().size(), true);

    for(auto &path: paths)
    {
        if(path.find(keyword(WILDCARD)) == std::string::npos)
        {
            m_target_paths.push_back(path);
        }
        else
        {
            size_t pos = 0, last_pos = 0;
            std::vector<std::string> split_path;
            while((pos = path.find_first_of('.', last_pos)) != std::string::npos)
            {
                split_path.push_back(path.substr(last_pos, pos-last_pos));
                last_pos = pos+1;
            }

            split_path.push_back(path.substr(last_pos, pos-last_pos));

            auto target_paths = path_strings(split_path, document);

            for(auto &target_path: target_paths)
            {
                m_target_paths.push_back(target_path);
            }
        }
    }
}

void Projection::parse_next(json::Writer &writer)
{
    auto current = path_string(m_current_path);
    bool on_path = false;
    bool on_target = false;

    for(auto &target_path: m_target_paths)
    {
        auto len = std::min(current.size(), target_path.size());

        if(len == 0)
        {
            if(target_path.size() == len)
            {
                on_path = on_target = true;
            }
            else
            {
                on_path = true;
            }
        }
        else if(target_path.compare(0, len, current) == 0)
        {
            if(target_path.size() == len)
            {
                on_path = on_target = true;
            }
            else if(target_path[len] == '.')
            {
                on_path = true;
            }
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

        std::string nkey = m_write_path ? key : "";
        writer.write_raw_data(nkey, &m_view.data()[start], end-start);

        m_found_count += 1;
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
        parse_map(writer);
        break;
    case ObjectType::Array:
        parse_array(writer);
        break;
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

void Projection::parse_map(json::Writer &writer)
{
    uint32_t byte_size = 0;
    m_view >> byte_size;

    uint32_t size = 0;
    m_view >> size;

    std::string key = m_current_path.empty() ? "" : m_current_path.back();

    if(m_write_path)
    {
        writer.start_map(key);
    }

    for(uint32_t i = 0; i < size; ++i)
    {
        std::string key;
        m_view >> key;

        m_current_path.push_back(key);
        parse_next(writer);
        m_current_path.pop_back();
    }

    if(m_write_path)
    {
        writer.end_map();
    }
}

void Projection::parse_array(json::Writer &writer)
{
    uint32_t byte_size = 0;
    m_view >> byte_size;

    uint32_t size = 0;
    m_view >> size;

    std::string key = m_current_path.empty() ? "" : m_current_path.back();

    if(m_write_path)
    {
        writer.start_array(key);
    }

    for(uint32_t i = 0; i < size; ++i)
    {
        m_current_path.push_back(to_string(i));
        parse_next(writer);
        m_current_path.pop_back();
    }

    if(m_write_path)
    {
        writer.end_array();
    }
}


}
