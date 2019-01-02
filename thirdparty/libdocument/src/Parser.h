#pragma once

#include "json/json.h"

namespace json
{

/**
 * Helper class to convert a JSON string to a binary object
 */
class Parser
{
public:
    Parser(const std::string &str_, bitstream &result_);

    void do_parse();

private:
    void parse(const std::string &key);

    void skip_whitespace();

    void parse_array(const std::string &key);
    void parse_null(const std::string &key);
    void parse_string(const std::string &key);
    void parse_number(const std::string &key);
    void parse_map(const std::string &key);
    void parse_true(const std::string &key);
    void parse_false(const std::string &key);
    void parse_datetime(const std::string &key);

    std::string read_string();

    bool check_string(const std::string &value);

    const std::string &str;
    std::string::const_iterator it;

    Writer writer;
};

inline void Parser::skip_whitespace()
{
    while(*it == ' ' || *it == '\n' || *it == '\t')
    {
        ++it;
    }
}


}
