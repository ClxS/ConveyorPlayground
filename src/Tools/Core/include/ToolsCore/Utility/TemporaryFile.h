#pragma once
#include <filesystem>

namespace cpp_conv::tools::utility
{
    class TemporaryFile
    {
    public:
        TemporaryFile();
        TemporaryFile(std::string_view extension);
        ~TemporaryFile();

        TemporaryFile(const TemporaryFile&) = delete;
        TemporaryFile(const TemporaryFile&&) = delete;
        TemporaryFile operator=(const TemporaryFile&) = delete;
        TemporaryFile operator=(const TemporaryFile&&) = delete;

        [[nodiscard]] const std::filesystem::path& GetFile() const { return m_Path; }
    private:
        std::filesystem::path m_Path;
    };
}
