#pragma once


#include "json/json.h"

#include <string>
#include <vector>

namespace json
{

enum KeywordType
{
    IN, WILDCARD, FALSE, TRUE, NIL, LESS_THAN, LESS_THAN_EQUAL, GREATER_THAN, GREATER_THAN_EQUAL, EQUAL, NOT_EQUAL
};

constexpr const char* keyword(KeywordType type)
{
    if(type == TRUE)
    {
        return "true";
    }
    else if(type == FALSE)
    {
        return "false";
    }
    else if(type == WILDCARD)
    {
        return "*";
    }
    else if(type == NIL)
    {
        return "null";
    }
    else if(type == IN)
    {
        return "$in";
    }
    else if(type == LESS_THAN)
    {
        return "$lt";
    }
    else if(type == LESS_THAN_EQUAL)
    {
        return "$lte";
    }
    else if(type == GREATER_THAN)
    {
        return "$gt";
    }
    else if(type == GREATER_THAN_EQUAL)
    {
        return "$gte";
    }
    else if(type == EQUAL)
    {
        return "$eq";
    }
    else if(type == NOT_EQUAL)
    {
        return "$neq";
    }
    else
    {
        return "";
    }
}

/**
 * Expand paths containing wildcards accordingly
 */
inline std::vector<std::string> path_strings(const std::vector<std::string>& path, const json::Document &doc, const std::string &current_path = "", uint32_t it = 0)
{
    if(it == path.size())
    {
        std::vector<std::string> result;
        result.push_back(current_path);
        return result;
    }

    const std::string& str = path[it];
    ++it;

    if(str.compare(keyword(WILDCARD)) == 0)
    {
        std::vector<std::string> result;

        json::Document array_view(doc, current_path);
        if(array_view.get_type() != ObjectType::Array)
            return std::vector<std::string>();

        uint32_t size = array_view.get_size();

        for(uint32_t i = 0; i < size; ++i)
        {
            std::string spath = current_path + "." + std::to_string(i);
            auto ps = path_strings(path, doc, spath, it);

            for(auto &p : ps)
            {
                result.push_back(p);
            }
        }

        return result;
    }
    else
    {
        std::string spath = (current_path == "") ? str : current_path + "." + str;
        return path_strings(path, doc, spath, it);
    }
}

inline std::string path_string(const std::vector<std::string>& path)
{
    std::string result = "";

    for(uint32_t i = 0; i < path.size(); ++i)
    {
        if(i > 0)
            result += '.';

        result += path[i];
    }

    return result;
}

}
