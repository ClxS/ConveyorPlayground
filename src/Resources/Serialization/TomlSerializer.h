#pragma once
#include "DataField.h"
#include "tomlcpp.hpp"

namespace cpp_conv
{
    struct TomlSerializer
    {
        struct Config
        {
            std::string m_RootTable;
        };

        static std::string Serialize(
            const Config& config,
            const std::vector<DataFieldBase*>& fields,
            std::string* outErrors = nullptr)
        {
            return "";
        }

        static bool TryDeserialize(
            const std::string& input,
            const Config& config,
            const std::vector<DataFieldBase*>& fields,
            std::string* outErrors = nullptr)
        {
            const auto [table, errors] = ::toml::parse(input);
            if (!table)
            {
                if (outErrors)
                {
                    *outErrors = std::format("Failed to read TOML: {}\n", errors);
                }

                return false;
            }

            const auto rootTable = table->getTable(config.m_RootTable);
            if (!rootTable)
            {
                if (outErrors)
                {
                    *outErrors = std::format("File did not contain a top level {} entry\n", config.m_RootTable);
                }

                return false;
            }

            bool bErrors = false;
            for(auto& field : fields)
            {
                if (!field->TryRead(rootTable.get()) && field->m_bIsRequired)
                {
                    bErrors = true;
                    if (outErrors)
                    {
                        if (outErrors->size() > 0)
                        {
                            *outErrors = std::format("{}Failed to read required field {}\n", *outErrors, field->m_FieldName);
                        }
                        else
                        {
                            *outErrors = std::format("Failed to read required field {}\n", field->m_FieldName);
                        }
                    }
                }
            }

            return !bErrors;
        }
    };
}

