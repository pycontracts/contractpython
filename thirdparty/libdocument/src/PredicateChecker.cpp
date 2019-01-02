#include "PredicateChecker.h"

#include "defines.h"
#include "json.h"

using std::to_string;

namespace json
{

PredicateChecker::PredicateChecker(const json::Document &document)
    : m_pred_matches(false), m_document(document), m_matched(true)
{
}

bool PredicateChecker::get_result() const
{
    return m_matched;
}

void PredicateChecker::push_path(const std::string &path)
{
    if(path.empty())
    {
        return;
    }

    size_t pos = 0;
    size_t last_pos = 0;
    size_t count = 1;

    // Split up condensed paths...
    while((pos = path.find_first_of('.', last_pos)) != std::string::npos)
    {
        const std::string key = path.substr(last_pos, pos-last_pos);
        last_pos = pos+1;

        push_key(key);
        count++;
    }

    const std::string last_key = path.substr(last_pos, std::string::npos);
    push_key(last_key);

    m_path_sizes.push(count);
}

void PredicateChecker::push_key(const std::string &key)
{
    if(key.empty())
    {
        throw json_error("cannot push empty key");
    }

    if(key == keyword(IN))
    {
        m_mode.push_back(predicate_mode::IN);

        m_pred_matches = false;
        m_pred_values.clear();

        for(auto &path: path_strings(m_path, m_document))
        {
            json::Document view(m_document, path, false);

            pred_value val;
            val.type = view.get_type();

            switch(val.type)
            {
            case ObjectType::Integer:
                val.integer = view.as_integer();
                break;
            case ObjectType::String:
                val.str = view.as_string();
                break;
            case ObjectType::Float:
                val.floating = view.as_float();
                break;
            default:
                val.type = ObjectType::Null;
                break;
            }

            m_pred_values.push_back(val);
        }
    }
    else if(key == keyword(LESS_THAN) || key == keyword(LESS_THAN_EQUAL)
            || key == keyword(GREATER_THAN) || key == keyword(GREATER_THAN_EQUAL)
            || key == keyword(EQUAL) || key == keyword(NOT_EQUAL))
    {
        if(key == keyword(LESS_THAN))
        {
            m_mode.push_back(predicate_mode::LESS_THAN);
        }
        else if(key == keyword(LESS_THAN_EQUAL))
        {
            m_mode.push_back(predicate_mode::LESS_THAN_EQUAL);
        }
        else if(key == keyword(GREATER_THAN))
        {
            m_mode.push_back(predicate_mode::GREATER_THAN);
        }
        else if(key == keyword(GREATER_THAN_EQUAL))
        {
            m_mode.push_back(predicate_mode::GREATER_THAN_EQUAL);
        }
        else if(key == keyword(EQUAL))
        {
            m_mode.push_back(predicate_mode::EQUAL);
        }
        else if(key == keyword(NOT_EQUAL))
        {
            m_mode.push_back(predicate_mode::NOT_EQUAL);
        }
        else
        {
            throw json_error("invalid state");
        }

        m_pred_values.clear();
        m_pred_matches = false;

        for(auto &path: path_strings(m_path, m_document))
        {
            json::Document view(m_document, path, false);

            pred_value val;
            val.type = view.get_type();

            switch(val.type)
            {
            case ObjectType::Integer:
                val.integer = view.as_integer();
                break;
            case ObjectType::Float:
                val.floating = view.as_float();
                break;
            case ObjectType::String:
                val.str = view.as_string();
                break;
            default:
                val.type = ObjectType::Null;
                break;
            }

            m_pred_values.push_back(val);
        }
    }
    else
    {
        m_mode.push_back(predicate_mode::NORMAL);
    }

    m_path.push_back(key);
}

void PredicateChecker::pop_path()
{
    if(m_path_sizes.empty())
    {
        return;
    }

    size_t count = m_path_sizes.top();
    m_path_sizes.pop();

    if(m_path.size() < count || m_path.size() != m_mode.size())
    {
        throw json_error("invalid state");
    }

    for(size_t i = 0; i < count; ++i)
    {
        if(m_mode.back() != predicate_mode::NORMAL)
        {
            if(!m_pred_matches)
            {
                m_matched = false;
            }
        }

        m_path.pop_back();
        m_mode.pop_back();
    }
}

void PredicateChecker::handle_binary(const std::string &key, const uint8_t *data, uint32_t len)
{   
    (void)key;
    (void)data;
    (void)len;
    //FIXME binary predicates?
}

void PredicateChecker::handle_string(const std::string &key, const std::string &value)
{
    push_path(key);

    if(mode() == predicate_mode::NORMAL)
    {
        bool found = false;

        for(auto &path: path_strings(m_path, m_document))
        {
            Document view(m_document, path, false);

            if(view.empty() || view.get_type() != ObjectType::String)
            {
                continue;
            }

            std::string other = view.as_string();
            if(other == value)
            {
                found = true;
            }
        }

        if(!found)
        {
            m_matched = false;
        }
    }
    else
    {
        if(mode() == predicate_mode::EQUAL)
        {
            for(auto &val: m_pred_values)
            {
                if(val.type == ObjectType::String && val.str == value)
                {
                    m_pred_matches = true;
                }
            }
        }
        else if(mode() == predicate_mode::IN)
        {
            for(auto &val: m_pred_values)
            {
                if(val.type == ObjectType::String && val.str == value)
                {
                    m_pred_matches = true;
                }
            }
        }
        else if(mode() == predicate_mode::NOT_EQUAL)
        {
            for(auto &val: m_pred_values)
            {
                if(val.type == ObjectType::String && val.str != value)
                {
                    m_pred_matches = true;
                }
            }
        }
     }

    pop_path();
}

void PredicateChecker::handle_integer(const std::string &key, const integer_t value)
{
    push_path(key);

    if(mode() == predicate_mode::NORMAL)
    {
        bool found = false;

        for(auto &path : path_strings(m_path, m_document))
        {
            Document view(m_document, path, false);

            if(view.empty() || view.get_type() != ObjectType::Integer)
            {
                continue;
            }

            integer_t other = view.as_integer();
            if(other == value)
            {
                found = true;
            }
        }

        if(!found)
        {
            m_matched = false;
        }
    }
    else if(mode() == predicate_mode::IN)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Integer && val.integer == value)
            {
                m_pred_matches = true;
            }
        }
    }
    else if(mode() == predicate_mode::LESS_THAN)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating < value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer < value)
            {
                m_pred_matches = true;
            }
        }
    }
    else if(mode() == predicate_mode::LESS_THAN_EQUAL)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating <= value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer <= value)
            {
                m_pred_matches = true;
            }
        }
    }
    else if(mode() == predicate_mode::GREATER_THAN)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating > value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer > value)
            {
                m_pred_matches = true;
            }
        }
    }
    else if(mode() == predicate_mode::GREATER_THAN_EQUAL)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating >= value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer >= value)
            {
                m_pred_matches = true;
            }
        }
    }
    else if(mode() == predicate_mode::EQUAL)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating == value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer == value)
            {
                m_pred_matches = true;
            }
        }
    }
    else if(mode() == predicate_mode::NOT_EQUAL)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating != value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer != value)
            {
                m_pred_matches = true;
            }
        }
    }
 
    pop_path();
}

void PredicateChecker::handle_float(const std::string &key, const json::float_t value)
{
    push_path(key);

    if(mode() == predicate_mode::NORMAL)
    {
        Document view(m_document, path_string(m_path), false);

        if(view.empty() || view.get_type() != ObjectType::Float)
        {
            m_matched = false;
        }
        else
        {
            json::float_t other = view.as_float();
            if(other != value)
            {
                m_matched = false;
            }
        }
    }
    else if(mode() == predicate_mode::IN)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating == value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer == value)
            {
                m_pred_matches = true;
            }
        }
    }
    else if(mode() == predicate_mode::LESS_THAN)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating < value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer < value)
            {
                m_pred_matches = true;
            }
        }
    }
    else if(mode() == predicate_mode::LESS_THAN_EQUAL)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating <= value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer <= value)
            {
                m_pred_matches = true;
            }
        }
    }
    else if(mode() == predicate_mode::GREATER_THAN)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating > value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer > value)
            {
                m_pred_matches = true;
            }
        }
    }
    else if(mode() == predicate_mode::GREATER_THAN_EQUAL)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating >= value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer >= value)
            {
                m_pred_matches = true;
            }
        }
    }
    else if(mode() == predicate_mode::EQUAL)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating == value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer == value)
            {
                m_pred_matches = true;
            }
        }
    }
    else if(mode() == predicate_mode::NOT_EQUAL)
    {
        for(auto &val: m_pred_values)
        {
            if(val.type == ObjectType::Float && val.floating != value)
            {
                m_pred_matches = true;
            }
            else if(val.type == ObjectType::Integer && val.integer != value)
            {
                m_pred_matches = true;
            }
        }
    }

    pop_path();
}

void PredicateChecker::handle_map_start(const std::string &key)
{
    push_path(key);
}

void PredicateChecker::handle_boolean(const std::string &key, const bool value)
{
    push_path(key);

    Document view(m_document, path_string(m_path), false);

    if(view.empty() || (view.get_type() != ObjectType::True && view.get_type() != ObjectType::False))
    {
        m_matched = false;
    }
    else
    {
        bool other = view.as_boolean();
        if(other != value)
        {
            m_matched = false;
        }
    }

    pop_path();
}

void PredicateChecker::handle_null(const std::string &key)
{
    //FIXME
    (void)key;
}

void PredicateChecker::handle_datetime(const std::string &key, const tm& value)
{
    //FIXME
    (void)key;
    (void)value;
}

void PredicateChecker::handle_map_end()
{
    pop_path();
}

void PredicateChecker::handle_array_start(const std::string &key)
{
    push_path(key);
}

void PredicateChecker::handle_array_end()
{
    pop_path();
}

PredicateChecker::predicate_mode PredicateChecker::mode() const
{
    for(auto &m : m_mode)
    {
        if(m != predicate_mode::NORMAL)
        {
            return m;
        }
    }

    return predicate_mode::NORMAL;
}

}
