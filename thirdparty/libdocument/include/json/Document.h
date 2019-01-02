#pragma once

#include <fstream>
#include <stdbitstream.h>

#include "json/defines.h"
#include "json/Diff.h"
#include "json/Iterator.h"

#ifdef USE_GEO
#include "geo/vector2.h"
#endif

namespace json
{

class Document
{
public:
    Document(Document &&other) noexcept;

    /**
     * Generate document by extracting a set of paths from an existing document
     */
    Document(const Document &parent, const std::vector<std::string> &paths, bool force = false);

    /**
     * Get a view of a specific child 
     */
    Document(const Document &parent, const uint32_t pos);

    /**
     * Create a view of a subset of the document
     */
    Document(const Document &parent, const std::string &path, bool force = false);

    /**
     * Creates an empty and invalid object
     */
    Document() = default;

    explicit Document(bitstream &data);

    /**
     * Creates an object from a JSON string
     */
    explicit Document(const std::string& str);

#ifndef IS_ENCLAVE
    /**
     * Load from a binary file
     */
    explicit Document(std::ifstream &file);
#endif

    Document(const uint8_t *data, uint32_t length, DocumentMode mode);
    Document(uint8_t *data, uint32_t length, DocumentMode mode);

    ~Document() = default;

    void assign(bitstream &&data)
    {
        m_content = std::move(data);
    }

    bool valid() const;

    bool empty() const
    {
        return get_type() == ObjectType::Null;
    }

    ObjectType get_type() const;

    /**
     * Get the number of children held by this object
     *
     * \note This is only supported for maps and arrays
     */
    uint32_t get_size() const;

#ifdef USE_GEO
    geo::vector2d as_vector2() const;
#endif

    std::string as_string() const;
    integer_t as_integer() const;
    float_t as_float() const;
    bool as_boolean() const;
    bitstream as_bitstream() const;

    /**
     * Add to or create the specified field
     *
     * Add semantics are only supported for numeric types
     */
    bool add(const std::string &path, const json::Document &value);
    
    /**
     * Get the key of the n-th child
     *
     * \note This is only supported for maps
     *
     * \throws invalid_argument if the position is out of bounds
     * \throws json_error if document is not a map
     */
    std::string get_key(size_t pos) const;
    
    /**
     * Returns a read-only view of the child as position pos
     *
     * \note This is only supported for arrays and maps
     *
     * \throws invalid_argument if the position is out of bounds
     * \throws json_error if document is not a map or array
     */
    json::Document get_child(size_t pos) const;

    bool matches_predicates(const json::Document &predicates) const;

    void iterate(Iterator &iterator) const;

    const bitstream& data() const
    {
        return m_content;
    }

    bitstream& data()
    {
        return m_content;
    }

    void operator=(Document &&other)
    {
        m_content = std::move(other.m_content);
    }

    /**
     * Discard contents of the document
     */
    void clear()
    {
        m_content.clear();
    }

    /**
     * Returns a 64-bit hash of the content of this document
     */
    int64_t hash() const;

    void detach_data(uint8_t* &data, uint32_t &len)
    {
        m_content.detach(data, len);
    }

    /**
     * Create an identical copy of this document
     *
     * \param force_copy
     *      Ensure the new document is operation on a new buffer
     */
    Document duplicate(bool force_copy = false) const;

    bool insert(const std::string &path, const json::Document &doc);

    void compress(bitstream &bstream) const;

    /**
     * Returns a compact human-readable JSON string that holds the contents of this document
     */
    std::string str() const;

    /**
     * Get a human-readable representation of the document that is nicely formatted
     */
    std::string pretty_str(int indent) const;

    /**
     *  Size in bytes of this document
     */
    size_t byte_size() const
    {
        return m_content.size();
    }

    Diffs diff(const Document &other) const;

protected:
    bitstream m_content;
};

/**
 * Construct a document from a std::string
 */
class String : public Document
{
public:
    String(const char *str);

    String(const std::string &str);

    void operator=(const std::string &str);
};

class Binary : public Document
{
public:
    Binary(uint8_t *data, uint32_t length)
        : Document()
    {
        m_content << ObjectType::Binary;
        m_content << length;
        m_content.write_raw_data(data, length);
        m_content.move_to(0);
    }

    Binary(bitstream &bstream)
        : Document()
    {
        m_content << ObjectType::Binary;
        m_content << static_cast<uint32_t>(bstream.size());
        m_content.write_raw_data(bstream.data(), bstream.size());
        m_content.move_to(0);
    }
};

/**
 * Construct a document from an integer
 */
class Integer : public Document
{
public:
    Integer(integer_t i);
};

inline bool operator==(const json::Document &first, const json::Document &second)
{
    return first.data() == second.data();
}

inline bool operator!=(const json::Document &first, const json::Document &second)
{
    return !(first == second);
}

}

inline bitstream& operator<<(bitstream &bs, const json::Document& doc)
{
    doc.compress(bs);
    return bs;
}

inline bitstream& operator>>(bitstream &bs, json::Document& doc)
{
    doc = json::Document(bs);
    return bs;
}

#ifndef IS_ENCLAVE
inline std::ostream& operator<<(std::ostream &os, const json::Document &doc)
{
    return os << doc.str();
}
#endif

