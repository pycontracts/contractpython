#include "Parser.h"
#include "json.h"

namespace json
{

inline json_error make_parse_error(const std::string &msg, const std::string &str, const std::string &expected, const std::string::const_iterator &it)
{
    std::string out;
    out += msg + ": <" + str + ">. ";
    out += "Expected \"" + expected + "\", got ";

    if(it == str.end())
    {
        out += "<EOS>";
    }
    else
    {
        out += '\"';
        out += *it;
        out += '\"';
    }

    return json_error(out);
}

Parser::Parser(const std::string& str_, bitstream &result_)
    : str(str_), writer(result_)
{
    it = str.begin();
}

void Parser::do_parse()
{
    parse("");
}

void Parser::parse(const std::string &key)
{
    skip_whitespace();

    if(it == str.end())
    {
        return;
    }

    switch (*it)
    {
    case '{':
        parse_map(key);
        break;
    case '[':
        parse_array(key);
        break;
    case '"':
        parse_string(key);
        break;
    case '+':
    case '-':
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'e':
    case 'E':
        parse_number(key);
        break;
    case 't': // STR_TRUE[0]:
        parse_true(key);
        break;
    case 'f': //STR_FALSE[0]:
        parse_false(key);
        break;
    case 'n':
        parse_null(key);
        break;
    case 'd':
        parse_datetime(key);
        break;
    default:
        throw json_error("Invalid JSON value");
    }
}

bool Parser::check_string(const std::string &value)
{
    size_t c = 0;

    while(c < value.size() && it != str.end())
    {
        if(value[c] != *it)
        {
            return false;
        }

        ++c;
        ++it;
    }

    return c == value.size();
}

void Parser::parse_datetime(const std::string &key)
{
    if(!check_string("d\""))
    {
        throw json_error("Not a datetime structure!");
    }

    enum class parse_state
    {
        Year,
        Month,
        Day,
        Hour,
        Minute,
        Second,
        Done
    };

    auto state = parse_state::Year;
    std::string temp;

    tm val;
    memset(&val, 0, sizeof(val));

    for(;it != str.end() && state != parse_state::Done; ++it)
    {
        if(isspace(*it) != 0 || *it == '-' || *it == ':' || *it == '"')
        {
            if(temp.empty())
            {
                continue;
            }

            const char *start = &temp[0];
            char *end = nullptr;
            auto i = std::strtol(start, &end, 10);

            switch(state)
            {
            case parse_state::Year:
                state = parse_state::Month;
                val.tm_year = i;
                break;
            case parse_state::Month:
                state = parse_state::Day;
                val.tm_mon = i;
                break;
            case parse_state::Day:
                state = parse_state::Hour;
                val.tm_mday = i;
                break;
            case parse_state::Hour:
                state = parse_state::Minute;
                val.tm_hour = i;
                break;
            case parse_state::Minute:
                state = parse_state::Second;
                val.tm_min = i;
                break;
            case parse_state::Second:
                state = parse_state::Done;
                val.tm_sec = i;
                break;
            default:
                throw json_error("unknown datetime parse state");
                break;
            }

            temp = "";
        }
        else
        {
            temp += *it;
        }
    }

    if(state != parse_state::Done)
    {
        throw json_error("Failed to parse datetime");
    }

    //TODO check/support for other formats
    //auto res = strptime(str.c_str(), "%Y-%m-%d %T", &val);
    //time_t since_epoch = timegm(&val);

    writer.write_datetime(key, val);
}

void Parser::parse_map(const std::string &key)
{
    if(it == str.end() || *it != '{')
    {
        throw make_parse_error("Not a valid map", str, "{", it);
    }

    ++it;

    writer.start_map(key);

    bool first = true;

    while(it != str.end() && *it != '}')
    {
        skip_whitespace();

        if(first)
        {
            first = false;
        }
        else
        {
            if(it == str.end() || *it != ',')
            {
                throw make_parse_error("Not a valid map", str, ",", it);
            }

            ++it;
            skip_whitespace();
        }

        std::string child_key = read_string();

        skip_whitespace();

        if(it == str.end() || *it != ':')
        {
            throw make_parse_error("Not a valid map", str, ":", it);
        }

        ++it;
        skip_whitespace();

        parse(child_key);

        skip_whitespace();
    }

    if(*it != '}')
    {
        throw json_error("Map not terminated!");
    }

    ++it;

    writer.end_map();
}

void Parser::parse_number(const std::string &key)
{
    bool is_double = false;

    std::string::const_iterator it2 = it;

    while (it2 != str.end())
    {
        if(!(isdigit(*it2) != 0 || *it2 == '+' ||
             *it2 == '-' || *it2 == '.' ||
             *it2 == 'e' || *it2 == 'E' ))
        {
            break;
        }

        if (*it2 == '.' || *it2 == 'e' || *it2 == 'E')
        {
            is_double = true;
        }

        ++it2;
    }

    const char *start = it.base();
    char *end = nullptr;

    if(is_double)
    {
        json::float_t val = std::strtod(start, &end);
        writer.write_float(key, val);
    }
    else
    {
        integer_t val = std::strtol(start, &end, 10);
        writer.write_integer(key, val);
    }

    size_t diff = end-start;

    for(size_t i = 0; i < diff; ++i)
    {
        ++it;
    }
}

void Parser::parse_true(const std::string &key)
{
    if(!check_string(keyword(TRUE)))
    {
        throw json_error("Not a valid boolean");
    }

    writer.write_boolean(key, true);
}

void Parser::parse_false(const std::string &key)
{
    if(!check_string(keyword(FALSE)))
    {
        throw json_error("Not a valid boolean");
    }

    writer.write_boolean(key, false);
}

void Parser::parse_null(const std::string &key)
{
    if(!check_string(keyword(NIL)))
    {
        throw json_error("Not a valid null value");
    }

    writer.write_null(key);
}

void Parser::parse_array(const std::string &key)
{
    if(it == str.end() || *it != '[')
    {
        throw json_error("Not a valid array!");
    }

    ++it;

    writer.start_array(key);
    bool first = true;

    uint32_t pos = 0;

    while(it != str.end() && *it != ']')
    {
        skip_whitespace();

        if(first)
        {
            first = false;
        }
        else
        {
            if(it == str.end() || *it != ',')
            {
                throw json_error("Not a valid array");
            }

            ++it;
            skip_whitespace();
        }

        parse(std::to_string(pos));
        pos++;

        skip_whitespace();
    }

    skip_whitespace();

    if(*it != ']')
    {
        throw json_error("Array not terminated!");
    }

    ++it;
    writer.end_array();
}

void Parser::parse_string(const std::string &key)
{
    auto str = read_string();
    writer.write_string(key, str);
}

std::string Parser::read_string()
{
    if(it == str.end() || *it != '"')
    {
        throw json_error("Not a valid string");
    }

    ++it;
    std::string result;

    while(it != str.end() && *it != '"')
    {
        result += *it;
        ++it;
    }

    if(it == str.end())
    {
        throw json_error("String not terminated!");
    }

    ++it;
    return result;
}

}
