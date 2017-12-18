#ifdef USE_GEO

#include "Geo.h"

#include <cow/unpack.h>
#include "geo/vector2.h"

using namespace cow;

namespace cow
{

ValuePtr GeoModule::get_member(const std::string &name)
{
    auto &mem = memory_manager();

    if(name == "vector2")
    {
        return make_value<Function>(mem,
                [&mem, this](const std::vector<ValuePtr> &args) -> ValuePtr
                {
                    if(args.size() != 2)
                        throw std::runtime_error("invalid number of arguments");

                    auto x = unpack_float(args[0]);
                    auto y = unpack_float(args[1]); 
                    geo::vector2d vec(x,y);

                    return make_value<cow::vector2>(mem, vec);
                });
    }
    else
        throw std::runtime_error("Can't get member");
}

}

#endif
