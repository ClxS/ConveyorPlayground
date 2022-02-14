#pragma once

enum class ExitCode
{
    Success = 0,
    ArgumentParseError      = 1,
    UnknownMode             = 2,
    MissingRequiredArgument = 3,
    FileOutputMismatch      = 4,
};
