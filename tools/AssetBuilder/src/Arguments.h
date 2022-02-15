#pragma once

#include <string>
#include "ToolsCore/ArgumentParser.h"

struct Arguments : public cppconv::tools::arg_parser::ArgumentsBase
{
    ARG_CTOR_5_V(Arguments, Mode, DataRoot, Platform, Namespace, OutputFile, UpdateOnlySpecification)

    VERB_2(Mode, "generate-spec", "cook");

    REQUIRED_ARG(DataRoot, std::string, 'r', "root", "The path to the root data folder");
    REQUIRED_ARG(Platform, std::string, 'p', "platform", "The platform to build");
    ARG(Namespace, std::string, 'n', "ns", "Namespace for outputted files/symbols");
    ARG(OutputFile, std::string, 'o', "output", "Path to the output file");
    ARG(UpdateOnlySpecification, bool, 'd', "spec-only", "If true, the builder will only generate the updated asset specification");
};
