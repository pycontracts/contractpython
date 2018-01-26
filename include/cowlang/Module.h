#pragma once

#include <memory>

#include "Function.h"

namespace cow
{

class Module;
typedef std::shared_ptr<Module> ModulePtr;
    
class Module : public Value
{
public:
    Module(MemoryManager &mem)
        : Value(mem)
    {}

    virtual ~Module() {}

    ValueType type() const override { return ValueType::Module; }

    ValuePtr duplicate(MemoryManager&) override
    {
        return nullptr; //not supported
    }
};

}
