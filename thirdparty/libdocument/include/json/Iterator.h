#pragma once

#include "defines.h"

namespace json
{
class Iterator
{
public:
    virtual ~Iterator() {}

    virtual void handle_string(const std::string &key, const std::string &str) = 0;
    virtual void handle_integer(const std::string &key, const integer_t value) = 0;
    virtual void handle_float(const std::string &key, const float_t value) = 0;
    virtual void handle_map_start(const std::string &key) = 0;
    virtual void handle_boolean(const std::string &key, const bool value) = 0;
    virtual void handle_null(const std::string &key) = 0;
    virtual void handle_map_end() = 0;
    virtual void handle_array_start(const std::string &key) = 0;
    virtual void handle_array_end() = 0;
    virtual void handle_binary(const std::string &key, const uint8_t *data, uint32_t size) = 0;
    virtual void handle_datetime(const std::string &key, const tm& value) = 0;
};
}
