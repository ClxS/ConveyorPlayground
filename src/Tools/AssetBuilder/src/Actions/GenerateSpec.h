#pragma once
#include "ExitCodes.h"

struct Arguments;

namespace asset_builder::actions
{
    ExitCode generateSpec(const Arguments& args);
}
