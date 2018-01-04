#pragma once

#include "Interpreter.h"
#include <json/json.h>

namespace cow
{

json::Document value_to_document(ValuePtr val);

bitstream compile_file(const std::string &filename);
bitstream compile_code(const std::string &code);

}
