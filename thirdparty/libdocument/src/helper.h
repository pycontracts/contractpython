#pragma once

#include <string>
#include "DocumentTraversal.h"

namespace json
{

class DocumentDiffs : public DocumentTraversal
{
public:
    DocumentDiffs(const bitstream &data1, const bitstream &data2)
    {
        view1.assign(const_cast<uint8_t*>(data1.data()), data1.size(), true);
        view2.assign(const_cast<uint8_t*>(data2.data()), data2.size(), true);
    }

    void create_diffs(Diffs &diffs)
    {
        parse_next(diffs, false);
    }

private:
    void parse_next(Diffs &diffs, bool inside_diff)
    {
        if(path1 != path2)
        {
            throw json_error("Invalid state");
        }

        uint32_t start = view2.pos();

        ObjectType type1, type2;
        view1 >> type1;
        view2 >> type2;

        if(type1 != type2)
        {
            skip_next(type1, view1);
            skip_next(type2, view2);
            uint32_t end = view2.pos();

            diffs.emplace_back(Diff(DiffType::Modified, path_string(path1), &view2.data()[start], end-start));
        }
        else
        {
            switch(type1)
            {
            case ObjectType::String:
            {
                std::string str1, str2;
                view1 >> str1;
                view2 >> str2;

                uint32_t end = view2.pos();

                if(!inside_diff && str1 != str2)
                {
                    diffs.emplace_back(Diff(DiffType::Modified, path_string(path1), &view2.data()[start], end-start));
                }
                break;
            }
            case ObjectType::Integer:
            {
                integer_t i1, i2;
                view1 >> i1;
                view2 >> i2;

                uint32_t end = view2.pos();

                if(!inside_diff && i1 != i2)
                {
                    diffs.emplace_back(Diff(DiffType::Modified, path_string(path1), &view2.data()[start], end-start));
                }
                break;
            }
            case ObjectType::Float:
            {
                json::float_t d1, d2;
                view1 >> d1;
                view2 >> d2;

                uint32_t end = view2.pos();

                if(!inside_diff && d1 != d2)
                {
                    diffs.emplace_back(Diff(DiffType::Modified, path_string(path1), &view2.data()[start], end-start));
                }
                break;
            }
            case ObjectType::Map:
                parse_map(diffs, inside_diff);
                break;
            case ObjectType::Array:
                parse_array(diffs, inside_diff);
                break;
            case ObjectType::True:
            case ObjectType::False:
            case ObjectType::Null:
                break;
            default:
                throw json_error("Document diff failed: unknown object type");
            }
        }
    }

private:
    bitstream view1, view2;
    std::vector<std::string> path1;
    std::vector<std::string> path2;

    void parse_map(Diffs &diffs, bool inside_diff)
    {
        uint32_t byte_size1 = 0, byte_size2 = 0;
        view1 >> byte_size1;
        view2 >> byte_size2;

        uint32_t size1 = 0, size2 = 0;
        view1 >> size1;
        view2 >> size2;

        uint32_t i = 0, j = 0;

        while(i < size1 || j < size2)
        {
            std::string key1, key2;

            if(i < size1)
            {
                view1 >> key1;
                path1.push_back(key1);
            }

            if(j < size2)
            {
                view2 >> key2;
                path2.push_back(key2);
            }

            if(key1 == key2)
            {
                if(key1.empty())
                {
                    throw json_error("Invalid state");
                }
                
                parse_next(diffs, inside_diff);
            }
            else
            {
                //FIXME what if entries moved around?

                if(i < size1)
                {
                    uint32_t start = view1.pos();
                    ObjectType type;
                    view1 >> type;

                    skip_next(type, view1);
                    uint32_t end = view1.pos();

                    diffs.emplace_back(Diff(DiffType::Deleted, path_string(path1), &view1.data()[start], end-start));
                }

                if(j < size2)
                {
                    uint32_t start = view2.pos();
                    ObjectType type;
                    view2 >> type;

                    skip_next(type, view2);
                    uint32_t end = view2.pos();

                    diffs.emplace_back(Diff(DiffType::Added, path_string(path2), &view2.data()[start], end-start));
                }
            }

            if(i < size1)
            {
                path1.pop_back();
                ++i;
            }
            if(j < size2)
            {
                path2.pop_back();
                ++j;
            }
        }
    }

    void parse_array(Diffs &diffs, bool inside_diff)
    {
        uint32_t byte_size1 = 0, byte_size2 = 0;
        view1 >> byte_size1;
        view2 >> byte_size2;

        uint32_t size1 = 0, size2 = 0;
        view1 >> size1;
        view2 >> size2;

        uint32_t i = 0, j = 0;

        while(i < size1 || j < size2)
        {
            bool has_first = false, has_second = false;

            if(i < size1)
            {
                has_first = true;
                path1.push_back(std::to_string(i));
            }

            if(j < size2)
            {
                has_second = true;
                path2.push_back(std::to_string(j));
            }

            if(i == j && has_first && has_second)
            {
                parse_next(diffs, inside_diff);
            }
            else
            {
                if(has_first)
                {
                    uint32_t start = view1.pos();
                    ObjectType type;
                    view1 >> type;

                    skip_next(type, view1);
                    uint32_t end = view1.pos();

                    diffs.emplace_back(Diff(DiffType::Deleted, path_string(path1), &view1.data()[start], end-start));
                }

                if(has_second)
                {
                    uint32_t start = view2.pos();
                    ObjectType type;
                    view2 >> type;

                    skip_next(type, view2);
                    uint32_t end = view2.pos();

                    diffs.emplace_back(Diff(DiffType::Added, path_string(path2), &view2.data()[start], end-start));
                }
            }

            if(has_first)
            {
                path1.pop_back();
                ++i;
            }
            if(has_second)
            {
                path2.pop_back();
                ++j;
            }
        }
    }
};

class DocumentAdd : public DocumentTraversal
{
public:
    DocumentAdd(bitstream &data, const std::string &path, const json::Document &value)
        : m_view(data), m_target_path(path), m_value(value)
    {
        m_view.move_to(0);
    }

    void do_add()
    {
        auto t = m_value.get_type();

        if(t != ObjectType::Integer && t != ObjectType::Float && t != ObjectType::String)
        {
            throw json_error("Add not defined on type type!");
        }

        parse_next();
    }

private:
    void parse_next()
    {
        auto current = path_string(m_current_path);
        bool on_path = false;
        bool on_target = false;

        auto len = std::min(current.size(), m_target_path.size());
        if(m_target_path.compare(0, len, current) == 0)
        {
            on_path = true;
            if(current.size() >= m_target_path.size())
                on_target = true;
        }

        std::string key = "";
        if(m_current_path.size() > 0)
        {
            key = m_current_path.back();
        }

        ObjectType type;
        m_view >> type;

        if(!on_path)
        {
            skip_next(type, m_view);
            return;
        }

        if(on_target)
        {
            if(type == ObjectType::Integer)
            {
                static_assert(sizeof(json::integer_t) == sizeof(json::float_t), "integer and float must have same size");

                json::integer_t value;
                m_view >> value;

                if(m_value.get_type() == ObjectType::Integer)
                {
                    value += m_value.as_integer();
                    m_view.move_by(-1*static_cast<int32_t>(sizeof(value)));
                    m_view << value;
                }
                else if(m_value.get_type() == ObjectType::Float)
                {
                    auto res = value +  m_value.as_float();
                    m_view.move_by(-1 * static_cast<int32_t>(sizeof(ObjectType)+sizeof(value)));
                    m_view << ObjectType::Float << res;
                }
                else
                {
                    throw json_error("Cannot add: incompatible types");
                }
            }
            else if(type == ObjectType::Float)
            {
                json::float_t value;
                m_view >> value;

                if(m_value.get_type() == ObjectType::Integer)
                {
                    value += m_value.as_integer();
                    m_view.move_by(-1*static_cast<int32_t>(sizeof(value)));
                    m_view << value;
                }
                else if(m_value.get_type() == ObjectType::Float)
                {
                    value += m_value.as_float();
                    m_view.move_by(-1*static_cast<int32_t>(sizeof(value)));
                    m_view << value;
                }
                else
                {
                    throw json_error("Cannot add: incompatible types");
                }
 
            }
            else
            {
                throw json_error("Cannot add: Incompatible types");
            }
        }
        else
        {
            switch(type)
            {
            case ObjectType::String:
            case ObjectType::Integer:
            case ObjectType::Float:
                throw json_error("Invalid path");
                break;
            case ObjectType::Map:
                parse_map();
                break;
            case ObjectType::Array:
                parse_array();
                break;
            case ObjectType::True:
            case ObjectType::False:
            case ObjectType::Null:
                break;
            default:
                throw json_error("Document add failed: unknown object type");
            }
        }
    }

private:
    bitstream& m_view;
    const std::string m_target_path;
    std::vector<std::string> m_current_path;
    const json::Document &m_value;

    void parse_map()
    {
        uint32_t byte_size = 0;
        m_view >> byte_size;

        uint32_t size = 0;
        m_view >> size;

        for(uint32_t i = 0; i < size; ++i)
        {
            std::string key;
            m_view >> key;

            m_current_path.push_back(key);
            parse_next();
            m_current_path.pop_back();
        }
    }

    void parse_array()
    {
        uint32_t byte_size = 0;
        m_view >> byte_size;

        uint32_t size = 0;
        m_view >> size;

        for(uint32_t i = 0; i < size; ++i)
        {
            m_current_path.push_back(std::to_string(i));
            parse_next();
            m_current_path.pop_back();
        }
    }
};

}
