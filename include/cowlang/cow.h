#pragma once

#include "Interpreter.h"
#include "pypa/parser/parser.hh"

namespace cow
{

json::Document value_to_document(const ValuePtr &val);

bitstream compile_file(const std::string &filename, std::function<void(pypa::Error)> &e);
bitstream compile_string(const std::string &code, std::function<void(pypa::Error)> &e);
bitstream compile_string(const std::string &code);

} // namespace cow
