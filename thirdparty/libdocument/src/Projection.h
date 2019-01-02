#pragma once

#include <json/json.h>

#include "DocumentTraversal.h"

namespace json
{

/**
 * Creates a new document by extracting a set of paths from an existing docment
 */
class Projection : public DocumentTraversal
{
public:
    Projection(const Document &document, const std::vector<std::string> &paths, bool write_path);

    uint32_t do_search(bitstream &result)
    {
        json::Writer writer(result);

        if(!m_document.empty())
        {
            parse_next(writer);
        }

        return m_found_count;
    }

private:
    void parse_next(json::Writer &writer);

private:
    const json::Document &m_document;

    bitstream m_view;
    
    std::vector<std::string> m_target_paths; //FIXME preserve order
    std::vector<std::string> m_current_path;

    const bool m_write_path;
    uint32_t m_found_count;

    void parse_map(json::Writer &writer);
    void parse_array(json::Writer &writer);
};


}
