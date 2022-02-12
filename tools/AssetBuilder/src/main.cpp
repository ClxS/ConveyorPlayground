#include <iostream>

#include "Arguments.h"
#include "ExitCodes.h"

int main(const int argc, char **argv)
{
    Arguments args;
    if (!args.TryRead(argc, argv))
    {
        std::cerr << "Failed to read arguments";
        return static_cast<int>(ExitCode::ArgumentParseError);
    }

    return 0;
}

