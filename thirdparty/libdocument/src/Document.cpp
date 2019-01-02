#include "json/json.h"
#include "helper.h"
#include "DocumentMerger.h"
#include "Search.h"
#include "Projection.h"
#include "Iterator.h"
#include "PredicateChecker.h"
#include "Parser.h"

#include <cctype>
#include <ctime>
#include <list>
#include <stack>

using std::to_string;

namespace json
{

inline void skip_child(bitstream &view)
{
    ObjectType ctype;
    view >> ctype;

    switch(ctype)
    {
#ifdef USE_GEO
    case ObjectType::Vector2:
        view.move_by(sizeof(geo::vector2d));
        break;
#endif
    case ObjectType::Binary:
    case ObjectType::Map:
    case ObjectType::Array:
    case ObjectType::String:
    {
        uint32_t byte_size;
        view >> byte_size;
        view.move_by(byte_size);
        break;
    }
    case ObjectType::Null:
    case ObjectType::True:
    case ObjectType::False:
        break;
    case ObjectType::Integer:
        view.move_by(sizeof(json::integer_t));
        break;
    case ObjectType::Float:
        view.move_by(sizeof(json::float_t));
        break;
    case ObjectType::Datetime:
        view.move_by(sizeof(tm));
        break;
    default:
        throw json_error("Unknown document type!");
    }
}

#ifndef IS_ENCLAVE
Document::Document(std::ifstream &file)
{
    m_content << file;
}
#endif

Document::Document(const std::string &str)
{
    if(str.empty())
    {
        m_content << ObjectType::Null;
    }
    else
    {
        Parser parser(str, m_content);
        parser.do_parse();
    }

    m_content.move_to(0);
}

Document::Document(Document &&other) noexcept
    : m_content(std::move(other.m_content))
{
}

Document::Document(bitstream &data)
{
    uint32_t size = 0;
    data >> size;
    if(size > 0)
    {
        m_content.write_raw_data(data.current(), size);
    }
    data.move_by(size);
    m_content.move_to(0);
}

bool Document::valid() const
{
    if(m_content.empty())
    {
        return false;
    }

    // Only check the top level element
    // Tradeoff between performance and safety
    bitstream view;
    view.assign(m_content.data(), m_content.size(), true);
    skip_child(view);

    return view.at_end();
}

bool Document::matches_predicates(const json::Document &predicates) const
{
    if(predicates.empty())
    {
        return true;
    }

    json::PredicateChecker checker(*this);
    json::IterationEngine engine(predicates.data(), checker);
    engine.run();

    return checker.get_result();
}

bool Document::insert(const std::string &path, const Document &doc)
{
    DocumentMerger merger(m_content, path, doc.m_content);
    return merger.do_merge();
}

int64_t Document::hash() const
{
    return m_content.hash();
}

Document Document::duplicate(bool force_copy) const
{
    json::Document out("");
    out.assign(m_content.duplicate(force_copy));
    return out;
}

void Document::compress(bitstream &bstream) const
{
    uint32_t size = m_content.size();
    bstream << size;

    if(size > 0)
    {
        bstream.write_raw_data(m_content.data(), m_content.size());
    }
}

void Document::iterate(json::Iterator &iterator) const
{
    json::IterationEngine engine(data(), iterator);
    engine.run();
}

std::string Document::str() const
{
    if(!valid())
    {
        return "";
    }

    json::Printer printer;
    iterate(printer);
    return printer.get_result();
}

std::string Document::pretty_str(int indent) const
{
    json::DocumentPrettyPrinter printer(indent);
    iterate(printer);
    return printer.get_result();
}

Document::Document(const Document& parent, const std::vector<std::string> &paths, bool force)
{
    Projection proj(parent, paths, true);
    uint32_t num_found = proj.do_search(m_content);

    if(num_found != paths.size() && force)
    {
        throw json_error("Not all paths were found");
    }
}

Document::Document(const Document &parent, const uint32_t pos)
{
    bitstream view;
    view.assign(parent.m_content.data(), parent.m_content.size(), true);

    ObjectType type;
    view >> type;

    if(type != ObjectType::Array)
    {
        throw json_error("Not an array");
    }

    uint32_t byte_size, size;
    view >> byte_size >> size;

    if(pos >= size)
    {
        throw json_error("out of array bounds!");
    }

    for(uint32_t i = 0; i < pos; ++i)
    {
        ObjectType ot;
        view >> ot;
        DocumentTraversal::skip_next(ot, view);
    }

    auto start = view.current();

    ObjectType ot;
    view >> ot;
    DocumentTraversal::skip_next(ot, view);
    auto end = view.current();

    m_content.assign(start, end-start, true);
}

Document::Document(const Document& parent, const std::string &path, bool force)
{
    /// Wildcard may match multiple paths so we need to do a full projection
    if(path.find(keyword(WILDCARD)) != std::string::npos)
    {
        std::vector<std::string> paths = {path};

        Projection proj(parent, paths, true);
        uint32_t num_found = proj.do_search(m_content);

        if(num_found != paths.size() && force)
        {
            throw json_error("Not all paths were found");
        }
    }
    else
    {
        Search search(parent, path);
        bool success = search.do_search();

        if(!success && force)
        {
            throw json_error("Path was not found");
        }

        m_content = search.get_result();
    }
}

Document::Document(uint8_t *data, uint32_t length, DocumentMode mode)
{
    if(mode == DocumentMode::ReadOnly)
    {
        m_content.assign(data, length, true);
    }
    else if(mode == DocumentMode::ReadWrite)
    {
        m_content.assign(data, length, false);
    }
    else if(mode == DocumentMode::Copy)
    {
        if(length > 0)
        {
            m_content.write_raw_data(data, length);
        }
    }
    else
    {
        throw std::invalid_argument("Unknown Doucment mode");
    }

    m_content.move_to(0);
}

Document::Document(const uint8_t *data, uint32_t length, DocumentMode mode)
{
    if(mode == DocumentMode::ReadWrite)
    {
        throw std::invalid_argument("Cannot modify read-only data");
    }
    else if(mode == DocumentMode::ReadOnly)
    {
        m_content.assign(data, length, true);
    }
    else if(mode == DocumentMode::Copy)
    {
        if(length > 0)
        {
            m_content.write_raw_data(data, length);
        }
    }
    else
    {
        throw json_error("Unknown Doucment mode");
    }

    m_content.move_to(0);
}

Diff::Diff(DiffType type, const std::string &path, const uint8_t *value, uint32_t length)
{
    m_content << type << path << length;

    if(length > 0)
    {
        m_content.write_raw_data(value, length);
    }
}

bool Document::add(const std::string &path, const json::Document &value)
{
    DocumentAdd adder(m_content, path, value);

    try
    {
        adder.do_add();
        m_content.move_to(0);
        return true;
    }
    catch(json_error& e)
    {
        m_content.move_to(0);
        return false;
    }
}

json::Document Document::get_child(size_t pos) const
{
    bitstream view;
    view.assign(m_content.data(), m_content.size(), true);

    std::vector<std::string> result;

    ObjectType type;
    view >> type;

    if(type != ObjectType::Map && type != ObjectType::Array)
    {
        throw json_error("Document is not a map or array");
    }

    uint32_t byte_size, size;
    view >> byte_size >> size;

    if(pos >= size)
    {
        throw std::invalid_argument("Position is out of bounds!");
    }

    for(uint32_t i = 0; i < size; ++i)
    {
        if(type == ObjectType::Map)
        {
            std::string key;
            view >> key;
        }

        if(i == pos)
        {
            break;
        }
        else
        {
            skip_child(view);
        }
    }

    return json::Document(view.current(), view.remaining_size(), DocumentMode::ReadOnly);
}

std::string Document::get_key(size_t pos) const
{
    bitstream view;
    view.assign(m_content.data(), m_content.size(), true);

    ObjectType type;
    view >> type;

    if(type != ObjectType::Map)
    {
        throw json_error("Document is not a map");
    }

    uint32_t byte_size, size;
    view >> byte_size >> size;

    if(pos >= size)
    {
        throw std::invalid_argument("Position is out of bounds!");
    }

    for(uint32_t i = 0; i < size; ++i)
    {
        if(i == pos)
        {
            break;
        }
        else
        {
            std::string key;
            view >> key;

            skip_child(view);
        }
    }

    std::string key;
    view >> key;
    return key;
}

uint32_t Document::get_size() const
{
    bitstream view;
    view.assign(m_content.data(), m_content.size(), true);

    ObjectType type;
    view >> type;

    switch(type)
    {
    case ObjectType::Map:
    case ObjectType::Array:
    {
        uint32_t byte_size, size;
        view >> byte_size >> size;
        return size;
    }
    default:
        throw json_error("Object is not a map or array!");
    }
}

ObjectType Document::get_type() const
{
    bitstream view;
    view.assign(m_content.data(), m_content.size(), true);

    if(view.at_end())
    {
        return ObjectType::Null;
    }

    ObjectType type;
    view >> type;

    return type;
}

bitstream Document::as_bitstream() const
{
    bitstream view;
    view.assign(m_content.data(), m_content.size(), true);

    ObjectType type;
    view >> type;

    if(type != ObjectType::Binary)
    {
        throw json_error("Not a binary object");
    }

    uint32_t size;
    view >> size;

    bitstream result;
    result.assign(view.current(), size, true);
    return result;
}
/*
const uint8_t* Document::as_binary() const
{
    bitstream view;
    view.assign(m_content.data(), m_content.size(), true);

    ObjectType type;
    view >> type;

    if(type != ObjectType::Binary)
        throw json_error("Not a binary object");

    return view.current();
}*/

json::integer_t Document::as_integer() const
{
    bitstream view;
    view.assign(m_content.data(), m_content.size(), true);

    ObjectType type;
    view >> type;

    if(type != ObjectType::Integer)
    {
        throw json_error("Not an integer!");
    }

    json::integer_t i;
    view >> i;
    return i;
}

json::float_t Document::as_float() const
{
    bitstream view;
    view.assign(m_content.data(), m_content.size(), true);

    ObjectType type;
    view >> type;

    if(type != ObjectType::Float)
    {
        throw json_error("Not a float!");
    }

    json::float_t f;
    view >> f;
    return f;
}

bool Document::as_boolean() const
{
    bitstream view;
    view.assign(m_content.data(), m_content.size(), true);

    ObjectType type;
    view >> type;

    if(type == ObjectType::True)
    {
        return true;
    }
    else if(type == ObjectType::False)
    {
        return false;
    }
    else
    {
        throw json_error("Not a boolean!");
    }
}

#ifdef USE_GEO
geo::vector2d Document::as_vector2() const
{
    bitstream view;

    view.assign(m_content.data(), m_content.size(), true);

    ObjectType type;
    view >> type;

    if(type != ObjectType::Vector2)
    {
        throw json_error("Not a vector");
    }

    geo::vector2d res;
    view >> res.X;
    view >> res.Y;

    return res;
}
#endif

std::string Document::as_string() const
{
    bitstream view;
    view.assign(m_content.data(), m_content.size(), true);

    ObjectType type;
    view >> type;

    if(type != ObjectType::String)
    {
        throw json_error("Not a string!");
    }

    std::string str;
    view >> str;
    return str;
}

Diffs Document::diff(const Document &other) const
{
    Diffs diffs;
    DocumentDiffs runner(m_content, other.m_content);
    runner.create_diffs(diffs);
    return diffs;
}

json::Document Diff::as_document() const
{
    bitstream bstream;
    compress(bstream, false);

    uint8_t *data = nullptr;
    uint32_t len = 0;

    bstream.detach(data, len);

    return json::Document(data, len, json::DocumentMode::ReadWrite);
}

void Diff::compress(bitstream &bstream, bool write_size) const
{
    bitstream view;
    view.assign(m_content.data(), m_content.size(), true);

    uint32_t size = 0;
    auto size_pos = bstream.pos();

    if(write_size)
    {
        bstream << size;
    }

    Writer writer(bstream);

    writer.start_map("");
    DiffType type;
    view >> type;

    if(type == DiffType::Modified)
    {
        writer.write_string("type", "modified");
    }
    else if(type == DiffType::Deleted)
    {
        writer.write_string("type", "deleted");
    }
    else if(type == DiffType::Added)
    {
        writer.write_string("type", "added");
    }
    else
    {
        throw json_error("Unknown diff type");
    }

    std::string path;
    view >> path;
    writer.write_string("path", path);

    if(type == DiffType::Modified || type == DiffType::Added)
    {
        auto key = "value";
        if(type == DiffType::Modified)
        {
            key = "new_value";
        }

        uint32_t len = 0;
        view >> len;

        Document doc(view.current(), len, DocumentMode::ReadOnly);
        writer.write_document(key, doc);
    }

    writer.end_map();

    if(write_size)
    {
        auto end = bstream.size();
        bstream.move_to(size_pos);
        size = end - (size_pos + sizeof(size));
        bstream << size;
        bstream.move_to(end);
    }
}

String::String(const std::string &str)
{
    *this = str;
}

String::String(const char* string)
{
    std::string str = string;
    *this = str;
}

void String::operator=(const std::string &str)
{
    uint32_t length = str.size();
    m_content << ObjectType::String;
    m_content << length;
    if(length > 0)
    {
        m_content.write_raw_data(reinterpret_cast<const uint8_t*>(str.c_str()), length);
    }
    m_content.move_to(0);
}

Integer::Integer(const integer_t i)
{
    m_content << ObjectType::Integer;
    m_content << i;
    m_content.move_to(0);
}


}
