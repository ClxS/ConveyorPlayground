#include <iostream>

#include "Arguments.h"
#include "ExitCodes.h"
#include "ToolsCore/Hashing.h"

int doSpecGenerate(const Arguments&);

int main(const int argc, char **argv)
{
    Arguments args;
    if (!args.TryRead(argc, argv))
    {
        std::cerr << "Failed to read arguments";
        return static_cast<int>(ExitCode::ArgumentParseError);
    }

    using cppconv::tools::hashing::fnv1;

    switch (fnv1(args.m_Mode.m_Value))
    {
        case fnv1("generate-spec"): return doSpecGenerate(args);
    }

    return 0;
}

int doSpecGenerate(const Arguments& args)
{
    return 0;
}

