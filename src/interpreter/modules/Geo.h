#pragma once

#ifdef USE_GEO

#include <cowlang/cow.h>

namespace cow
{

class GeoModule : public cow::Module
{
public:
    GeoModule(cow::MemoryManager &mem) 
        : cow::Module(mem)
    {}

    cow::ValuePtr get_member(const std::string &name) override;
};

}

#endif
