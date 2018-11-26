#pragma once

#include "Interpreter.h"
#include <json/json.h>

namespace cow
{

json::Document value_to_document(const ValuePtr& val);

bitstream compile_file(const std::string &filename);
bitstream compile_string(const std::string &code);

}
