#include <stdbitstream.h>
#include "DocumentTraversal.h"

namespace json
{

class DocumentMerger : public DocumentTraversal
{
public:
    DocumentMerger(bitstream &doc, std::string full_path, const bitstream &other)
        : m_doc(doc), m_other(other), m_success(false)
    {
        size_t it;
        path.push_back("");

        while((it = full_path.find('.')) != std::string::npos)
        {
            std::string next = full_path.substr(0, it);
            full_path = full_path.substr(it+1, std::string::npos);

            path.push_back(next);
        }

        path.push_back(full_path);
    }

    bool do_merge()
    {
        parse_next("");
        m_doc.move_to(0);
        return m_success;
    }

    /// Return value indicates that element was found
    /// Check m_success for successful completion of update
    bool parse_next(const std::string &key)
    {
        ObjectType type;
        m_doc >> type;

        if(path.size() > 0 && path.front() == key)
        {
            path.pop_front();
        }
        else
        {
            skip_next(type, m_doc);
            return false;
        }

        bool is_target = path.size() == 1;

        if(is_target)
        {
            if(type == ObjectType::Map)
            {
                uint32_t map_start = m_doc.pos();
                uint32_t byte_size, size;
                m_doc >> byte_size >> size;

                auto &key = path.front();
                bool found = false;

                for(uint32_t i = 0; i < size; ++i)
                {
                    uint32_t start = m_doc.pos();

                    std::string ckey;
                    m_doc >> ckey;
                    ObjectType type;
                    m_doc >> type;

                    if(ckey == key)
                    {
                        // remove field first
                        skip_next(type, m_doc);
                        uint32_t end = m_doc.pos();
                        m_doc.move_to(start);
                        m_doc.remove_space(end-start);
                        found = true;
                    }
                    else
                        skip_next(type, m_doc);
                }

                if(found)
                {
                    //update sizes
                    uint32_t map_end = m_doc.pos();
                    byte_size = map_end - map_start - sizeof(byte_size);
                    size -= 1;

                    m_doc.move_to(map_start);
                    m_doc << byte_size << size;
                    m_doc.move_to(map_end);
                }

                // Make sure field does not exist yet
                if(!is_valid_key(key))
                {
                    return true;
                }

                insert_into_map(key, m_other, map_start);
                m_success = true;
                return true;
            }
            else if(type == ObjectType::Array)
            {
                uint32_t array_start = m_doc.pos();
                uint32_t byte_size = 0, size = 0;
                m_doc >> byte_size >> size;

                auto &key = path.front();

                // One can only append to an array for now
                if(!is_array_insertion(key))
                {
                    m_doc.move_by(byte_size - sizeof(size));
                    return true;
                }

                uint32_t pos = 0;
                for(; pos < size; ++pos)
                {
                    ObjectType type;
                    m_doc >> type;
                    skip_next(type, m_doc);
                }

                uint32_t elem_start = m_doc.pos();

                m_doc.make_space(m_other.size());
                m_doc.write_raw_data(m_other.data(), m_other.size());

                byte_size += m_doc.pos() - elem_start;
                size += 1;

                m_doc.move_to(array_start);
                m_doc << byte_size << size;

                m_doc.move_by(byte_size - sizeof(size));
                m_success = true;
                return true;
            }
            else
                return true;
        }
        else
        {
            switch(type)
            {
            case ObjectType::Map:
                return parse_map();
                break;
            case ObjectType::Array:
                return parse_array();
                break;
            default:
                throw json_error("we shouldn't be here");
            }
        }
    }

private:
    void insert_into_map(const std::string& key, const bitstream &data, uint32_t start)
    {
        m_doc.make_space(data.size() + sizeof(uint32_t) + key.size());
        m_doc << key;
        m_doc.write_raw_data(data.data(), data.size());

        uint32_t end = m_doc.pos();
        uint32_t byte_size = end - start - sizeof(byte_size); // this is the start of the map not the element

        m_doc.move_to(start);

        uint32_t _, size;
        m_doc >> _ >> size;
        size += 1;

        m_doc.move_to(start);
        m_doc << byte_size << size;
        m_doc.move_by(byte_size - sizeof(size));
    }

    bool parse_map()
    {
        uint32_t start = m_doc.pos();

        uint32_t byte_size = 0;
        m_doc >> byte_size;

        uint32_t size = 0;
        m_doc >> size;

        bool found = false;

        for(uint32_t i = 0; i < size; ++i)
        {
            std::string key;
            m_doc >> key;

            if(parse_next(key))
                found = true;
        }

        if(found && m_success)
        {
            uint32_t end = m_doc.pos();
            byte_size = end-start-sizeof(byte_size);
            m_doc.move_to(start);
            m_doc << byte_size;
            m_doc.move_to(end);
        }
        else if(!found)
        {
            if(path.size() < 2)
            {
                throw json_error("Invalid state");
            }

            bitstream data;
            json::Writer writer(data);

            auto it = path.begin();
            ++it;

            if(is_array_insertion(*it))
            {
                writer.start_array("");
                writer.end_array();
            }
            else
            {
                writer.start_map("");
                writer.end_map();
            }

            insert_into_map(path.front(), data, start);

            // try again...
            m_doc.move_to(start);
            parse_map();
        }

        return true;
    }

    bool is_array_insertion(const std::string &str)
    {
        return str == "+";
    }

    bool parse_array()
    {
        uint32_t array_start = m_doc.pos();

        uint32_t byte_size = 0;
        m_doc >> byte_size;

        uint32_t size = 0;
        m_doc >> size;

        bool found = false;

        for(uint32_t i = 0; i < size; ++i)
        {
            if(parse_next(std::to_string(i)))
                found = true;
        }

        if(found && m_success)
        {
            uint32_t end = m_doc.pos();
            byte_size = end - array_start - sizeof(byte_size);

            m_doc.move_to(array_start);
            m_doc << byte_size;
            m_doc.move_to(end);
        }

        //if(!found)
        //FIXME

        return found;
    }

    std::list<std::string> path;

    bitstream &m_doc;
    const bitstream &m_other;
    bool m_success;
};

}
