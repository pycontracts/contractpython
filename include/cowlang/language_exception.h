#pragma once

#include <string>
#include <stdexcept>

namespace cow
{

class language_exception : public std::exception
{
public:
    language_exception(const std::string &what)
        : std::exception(), m_what(what)
    {}

    const char* what() const noexcept override
    {
        return m_what.c_str();
    }

private:
    const std::string m_what;
};

}
