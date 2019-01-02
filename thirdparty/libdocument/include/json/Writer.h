#pragma once

#ifdef USE_GEO
#include <geo/vector2.h>
#endif

#include "defines.h"
#include "bitstream.h"
#include "Document.h"

namespace json
{

class Writer
{
private:
    static constexpr const char *EMPTY_KEY = "";

public:
    Writer(bitstream &result);

    Writer();
    ~Writer();

    void start_map() { start_map(EMPTY_KEY); }
    void start_array() { start_array(EMPTY_KEY); }

    void write_null() { write_null(EMPTY_KEY); }
    void write_binary(const bitstream &value) { write_binary(EMPTY_KEY, value); }
    void write_boolean(const bool value) { write_boolean(EMPTY_KEY, value); }
    void write_datetime(const tm &value) { write_datetime(EMPTY_KEY, value); }
    void write_integer(const integer_t &value) { write_integer(EMPTY_KEY, value); }
    void write_string(const std::string &value) { write_string(EMPTY_KEY, value); }
    void write_float(const float_t &value) { write_float(EMPTY_KEY, value); }


    void start_map(const std::string &key);
    void end_map();

    void start_array(const std::string &key);
    void end_array();

    /// Write data that is already binary formatted.
    void write_raw_data(const std::string &key, const uint8_t *data, uint32_t size);

    void write_document(const std::string &key, const json::Document &other)
    {
        if(!other.valid())
        {
            throw std::runtime_error("not a valid document!");
        }

        write_raw_data(key, other.data().data(), other.data().size());
    }

    void write_null(const std::string &key);
    void write_binary(const std::string &key, const bitstream &value);
    void write_boolean(const std::string &key, const bool value);
    void write_datetime(const std::string &key, const tm &value);
    void write_integer(const std::string &key, const integer_t &value);
    void write_string(const std::string &key, const std::string &value);
    void write_float(const std::string &key, const float_t &value);

#ifdef USE_GEO
    void write_vector2(const std::string &key, const geo::vector2d &vec);
#endif

    json::Document make_document();
private:
    void handle_key(const std::string &key);
    void check_end();

    bitstream *m_result_ptr;
    bitstream &m_result;

    enum mode_t {IN_ARRAY, IN_MAP, DONE};

    std::stack<mode_t> m_mode;
    std::stack<uint32_t> m_starts;
    std::stack<uint32_t> m_sizes;
};

}
