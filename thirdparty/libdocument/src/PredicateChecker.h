#pragma once

#include "Iterator.h"

namespace json
{

class PredicateChecker : public Iterator
{
public:
    PredicateChecker(const json::Document &document);

    bool get_result() const;

    void push_path(const std::string &path);
    void push_key(const std::string &key);
    void pop_path();

    void handle_string(const std::string &key, const std::string &value) override;
    void handle_integer(const std::string &key, const integer_t value) override;
    void handle_float(const std::string &key, const json::float_t value) override;
    void handle_map_start(const std::string &key) override;
    void handle_boolean(const std::string &key, const bool value) override;
    void handle_null(const std::string &key) override;
    void handle_datetime(const std::string &key, const tm& value) override;
    void handle_map_end() override;
    void handle_array_start(const std::string &key) override;
    void handle_array_end() override;
    void handle_binary(const std::string& key, const uint8_t *data, uint32_t len) override;

private:
    enum class predicate_mode {NORMAL, IN, LESS_THAN, LESS_THAN_EQUAL, GREATER_THAN, GREATER_THAN_EQUAL, EQUAL, NOT_EQUAL};

    predicate_mode mode() const;

    bool m_pred_matches;

    struct pred_value
    {
        ObjectType type;

        union
        {
            json::integer_t integer;
            json::float_t floating;
        };

        std::string str;
    };

    std::vector<pred_value> m_pred_values;

    std::vector<predicate_mode> m_mode;
    std::vector<std::string> m_path;
    std::stack<size_t> m_path_sizes;

    const json::Document &m_document;
    bool m_matched;
};


}
