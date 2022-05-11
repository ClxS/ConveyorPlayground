#pragma once
#include <string>

namespace asset_builder::utility::process_utility
{
    int execute(const std::string& executable, const std::string& args, std::string& stdOut, std::string& stdErr);
}
