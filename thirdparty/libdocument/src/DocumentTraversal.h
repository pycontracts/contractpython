#pragma once

#include "json.h"

using std::to_string;

namespace json
{

class DocumentTraversal
{
public:
    static void skip_next(ObjectType type, bitstream &view)
    {
        switch(type)
        {
#ifdef USE_GEO
        case ObjectType::Vector2:
        {
            view.move_by(sizeof(geo::vector2d));
            break;
        }
#endif
        case ObjectType::Binary:
        {
            uint32_t size = 0;
            view >> size;
            view.move_by(size);
            break;
        }
        case ObjectType::Integer:
        {
            view.move_by(sizeof(json::integer_t));
            break;
        }
        case ObjectType::Float:
        {
            view.move_by(sizeof(json::float_t));
            break;
        }
        case ObjectType::String:
        case ObjectType::Map:
        case ObjectType::Array:
        {
            uint32_t byte_size;
            view >> byte_size;
            view.move_by(byte_size);
            break;
        }
        case ObjectType::Datetime:
        {
            view.move_by(sizeof(tm));
            break;
        }
        case ObjectType::True:
        case ObjectType::False:
        case ObjectType::Null:
            break;
        default:
            throw json_error("Document traversal failed: Unknown object type");
        }
    }
};

}
