#include "cowlang/cow.h"

#include <boost/python.hpp>
namespace py = boost::python;
using namespace cow;

struct BitstreamConverter
{
    static PyObject *convert(const bitstream &bs)
    {
        auto data = reinterpret_cast<const char*>(bs.data());
        auto size = static_cast<Py_ssize_t>(bs.size());

        return PyByteArray_FromStringAndSize(data, size);
    }
};

BOOST_PYTHON_MODULE(cowlang)
{
    py::to_python_converter<bitstream, BitstreamConverter>();

    py::docstring_options doc_options;
    doc_options.disable_signatures();

    py::def("compile_string", &compile_string, "");
}
