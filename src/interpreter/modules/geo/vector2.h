#pragma once
#ifdef USE_GEO

#include <cowlang/cow.h>
#include <geo/vector2.h>

namespace cow
{

class vector2 : public Value
{
public:
    vector2(MemoryManager &mem, const geo::vector2d &v)
        : Value(mem), m_vec(v)
    {}

    ValueType type() const
    {
        return ValueType::geo_Vector2;
    }

    ValuePtr duplicate(MemoryManager &mem) override
    {
        return make_value<vector2>(mem, m_vec);
    }

    const geo::vector2d& get() const
    {
        return m_vec;
    }

    std::string str() const override
    {
        return std::to_string(m_vec);
    }

    ValuePtr get_member(const std::string &name) override
    {
        auto &mem = memory_manager();

        if(name == "x")
            return mem.create_float(m_vec.X);
        else if(name == "y")
            return mem.create_float(m_vec.Y);
        else
            throw std::runtime_error("Failed to get member!");
    }

private:
    geo::vector2d m_vec;
};

}

#endif
