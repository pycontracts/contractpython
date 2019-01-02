#pragma once

#include <stdexcept>
#include <string>

class json_error: public std::exception
{
public:
    json_error(const std::string &msg)
        : m_msg(msg)
    {}

    const char* what() const noexcept override
    {
        return m_msg.c_str();
    }

private:
    const std::string m_msg;
};
