#include <iostream>

#include "Arguments.h"
#include "ExitCodes.h"
#include "ToolsCore/Hashing.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "Actions/Cook.h"
#include "Actions/GenerateSpec.h"
#include "Processing/AssetProcessor.h"

ExitCode mainImpl(const int argc, char **argv)
{
    const Arguments args;
    if (!args.TryRead(argc, argv))
    {
        std::cerr << "Failed to read arguments\n";
        return ExitCode::ArgumentParseError;
    }

    AssetProcessor::initDefaults();

    using cppconv::tools::hashing::fnv1;
    switch (fnv1(args.m_Mode.m_Value))
    {
        case fnv1("generate-spec"): return asset_builder::actions::generateSpec(args);
        case fnv1("cook"): return asset_builder::actions::cook(args);
        default:
            std::cerr << "Unknown verb " << args.m_Mode.m_Value << "\n";
            return ExitCode::UnknownMode;
    }
}

int main(const int argc, char **argv)
{
    return static_cast<int>(mainImpl(argc, argv));
}
