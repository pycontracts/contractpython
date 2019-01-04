#pragma once

#include "Value.h"
#include "Scope.h"

namespace cow
{

class Callable : public Value
{
protected:
    Callable(MemoryManager &mem)
        :  Value(mem)
    {}

public:
    virtual ValuePtr call(const std::vector<ValuePtr>& arg, Scope& scope) = 0;

    bool is_callable() const override
    {
        return true;
    }

};

}
