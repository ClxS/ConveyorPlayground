#pragma once

#include <string>
#include "ToolsCore/ArgumentParser.h"

struct Arguments : public cppconv::tools::arg_parser::ArgumentsBase
{
    ARG_CTOR_3_V(Arguments, Mode, DataRoot, Platform, UpdateOnlySpecification)

    VERB_1(Mode, "generate-spec");

    ARG(DataRoot, std::string, 'r', "root", "The path to the root data folder");
    ARG(Platform, std::string, 'p', "platform", "The platform to build");
    ARG(UpdateOnlySpecification, bool, '-d', "spec-only", "If true, the builder will only generate the updated asset specification");
};
