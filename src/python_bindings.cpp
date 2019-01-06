#include "cowlang/cow.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using namespace cow;

template <> struct py::detail::type_caster<bitstream>
{
public:
    PYBIND11_TYPE_CASTER(bitstream, _("bitstream"));

    static handle cast(const bitstream &bs, return_value_policy /* policy */, handle /* parent */)
    {
        auto data = reinterpret_cast<const char *>(bs.data());
        auto size = static_cast<Py_ssize_t>(bs.size());

        return PyByteArray_FromStringAndSize(data, size);
    }
};

PYBIND11_MODULE(cowlang, m) {}
