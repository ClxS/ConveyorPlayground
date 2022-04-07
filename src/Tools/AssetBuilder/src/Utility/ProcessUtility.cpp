#include "ProcessUtility.h"
#include <Windows.h>
#include <corecrt_io.h>
#include <format>
#include <iostream>
#include <thread>
#include <variant>

HANDLE createStdOutHandle()
{
    SECURITY_ATTRIBUTES saAttr;
    ZeroMemory(&saAttr, sizeof(saAttr));
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    HANDLE pipeHandle {};
    HANDLE pipeWriteHandle {};
    if (!CreatePipe(&pipeHandle, &pipeWriteHandle, &saAttr, 0))
    {
        // log error
        std::cerr << std::format("Failed to create stdout pipe. Error: {}", GetLastError());
        return {};
    }

    if (!SetHandleInformation(pipeHandle, HANDLE_FLAG_INHERIT, 0))
    {
        // log error
        std::cerr << std::format("Failed to create set pipe handle information. Error: {}", GetLastError());
        return {};
    }

    return pipeHandle;
}

std::tuple<HANDLE, HANDLE> createStdOutHandles()
{
    return {
        createStdOutHandle(),
        createStdOutHandle()
    };
}

int asset_builder::utility::process_utility::execute(const std::string& executable, const std::string& args,
    std::string& stdOut, std::string& stdErr)
{
    // additional information
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    auto [stdOutHandle, stdErrHandle] = createStdOutHandles();

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);
    if (stdOutHandle && stdErrHandle)
    {
        si.hStdOutput = stdOutHandle;
        si.hStdError = stdErrHandle;
        si.dwFlags |= STARTF_USESTDHANDLES;
    }

    std::string mutableArgs = args;

    // start the program up
    if (!CreateProcessA(
            executable.c_str(),
            mutableArgs.data(),
            nullptr, // Process handle not inheritable
            nullptr, // Thread handle not inheritable
            TRUE, // Set handle inheritance to FALSE
            NULL, // Opens file in a separate console
            nullptr, // Use parent's environment block
            nullptr, // Use parent's starting directory
            &si,
            &pi))
    {
        std::cerr << "CreateProcess failed " << GetLastError() << "\n";
        return -1;
    }

    WaitForSingleObject( pi.hProcess, INFINITE );

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(stdOutHandle);
    CloseHandle(stdErrHandle);

    return exitCode;
}
