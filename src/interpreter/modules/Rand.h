#pragma once

#include "cow/Module.h"

namespace cow
{

class RandModule : public Module
{
public:
    using Module::Module;

    ValuePtr get_member(const std::string &name)  override;
};

}
