#pragma once

#include <json/json.h>

#include "DocumentTraversal.h"

namespace json
{

/**
 * Creates a view of a subset of an existing document
 * Will not copy the data
 */
class Search : public DocumentTraversal
{
public:
    Search(const Document &document, const std::string &path);

    bool do_search()
    {
        if(!m_document.empty())
        {
            parse_next();
        }

        return m_success;
    }

    bitstream get_result()
    {
        return std::move(m_result);
    }

private:
    void parse_next();

private:
    const json::Document &m_document;

    bitstream m_view;
    bitstream m_result;
    std::string m_target_path;
    
    std::vector<std::string> m_current_path;

    bool m_success = false;

    void parse_map();
    void parse_array();

};


}
