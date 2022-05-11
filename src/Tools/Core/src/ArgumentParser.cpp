#include "ToolsCore/ArgumentParser.h"

#include <format>
#include <iostream>
#include <sstream>
#include <string>

bool cppconv::tools::arg_parser::TypedArgReader<bool>::Read(const std::string& value, void* pTargetVariable)
{
    const auto typedTarget = static_cast<bool*>(pTargetVariable);
    if (value == "false" || value == "f" || value == "FALSE" || value == "0")
    {
        *typedTarget = false;
    }
    else
    {
        *typedTarget = true;
    }

    return true;
}

bool cppconv::tools::arg_parser::TypedArgReader<int32_t>::Read(const std::string& value, void* pTargetVariable)
{
    const auto typedTarget = static_cast<int32_t*>(pTargetVariable);
    return false;
}

bool cppconv::tools::arg_parser::TypedArgReader<float>::Read(const std::string& value, void* pTargetVariable)
{
    const auto typedTarget = static_cast<float*>(pTargetVariable);
    return false;
}

bool cppconv::tools::arg_parser::TypedArgReader<std::string>::Read(const std::string& value, void* pTargetVariable)
{
    const auto typedTarget = static_cast<std::string*>(pTargetVariable);
    *typedTarget = value;
    return true;
}

bool cppconv::tools::arg_parser::TypedArgReader<std::vector<std::string>>::Read(const std::string& value, void* pTargetVariable)
{
    const auto typedTarget = static_cast<std::vector<std::string>*>(pTargetVariable);

    std::vector<std::string> values;
    std::string segment;
    std::stringstream test(value);
    while(std::getline(test, segment, ';'))
    {
        values.push_back(segment);
    }

    *typedTarget = values;
    return true;
}

bool cppconv::tools::arg_parser::ArgumentsBase::TryRead(const int argc, char** argv) const
{
    bool bIsError = false;
    std::vector<std::string> errors;

    int iParameterIdx = 1;
    while(iParameterIdx < argc)
    {
        std::string parameter = argv[iParameterIdx];

        if (parameter.empty())
        {
            ++iParameterIdx;
            continue;
        }

        // Special handling for verbs
        if (iParameterIdx == 1 && m_pVerb != nullptr)
        {
            if (parameter[0] != '-')
            {
                const std::string* pSelectedVerb = nullptr;
                for(const auto& allowedVerb : m_pVerb->m_ValidValues)
                {
                    if (allowedVerb == parameter)
                    {
                        pSelectedVerb = &parameter;
                    }
                }

                if (pSelectedVerb)
                {
                    m_pVerb->m_Value = parameter;
                }
                else
                {
                    bIsError = true;
                    errors.push_back(std::string("Invalid verb: ") + parameter);
                }

                ++iParameterIdx;
                continue;
            }
            else
            {
                m_pVerb->m_Value = m_pVerb->m_ValidValues[0];
            }
        }

        ArgumentParameterBase* currentField = nullptr;
        for(ArgumentParameterBase* field : m_ArgumentFields)
        {
            if ((parameter.size() > 1 && field->m_Switch == parameter[1]) || std::format("--{}", field->m_SwitchLong) == parameter)
            {
                currentField = field;
                break;
            }
        }

        ++iParameterIdx;
        if (currentField == nullptr)
        {
            errors.push_back(std::string("Unknown option: ") + parameter);
            bIsError = true;
            while(iParameterIdx < argc && argv[iParameterIdx][0] != '-')
            {
                ++iParameterIdx;
            }
        }
        else
        {
            const char* szValueEntry = iParameterIdx < (argc) ? argv[iParameterIdx] : nullptr;

            if (szValueEntry == nullptr || szValueEntry[0] == '-')
            {
                // Special handling for "flag fields" (bool), which can be set just by the presence of the flag
                if (currentField->m_bIsFlag)
                {
                    if (currentField->Read(""))
                    {
                        currentField->m_bIsSet = true;
                    }
                }
                else
                {
                    errors.push_back(std::format("Parameter {} had no value!", currentField->m_Name));
                    bIsError = true;
                }

                while(iParameterIdx < argc && argv[iParameterIdx][0] != '-')
                {
                    ++iParameterIdx;
                }
                continue;
            }

            ++iParameterIdx;
            if (!currentField->Read(szValueEntry))
            {
                errors.push_back(std::format("Parameter {} failed to read value {}", currentField->m_Name, szValueEntry));
                bIsError = true;
                continue;
            }
            else
            {
                currentField->m_bIsSet = true;
            }
        }
    }

    for(const ArgumentParameterBase* field : m_ArgumentFields)
    {
        if (field->m_bIsRequired && !field->m_bIsSet)
        {
            errors.push_back(std::format("Required parameter {} was not set", field->m_Name));
            bIsError = true;
        }
    }

    if (!errors.empty())
    {
        if (bIsError)
        {
            std::cerr << "Argument parsing encountered some errors\n";
        }
        else
        {
            std::cout << "Argument parsing encountered some warnings\n";
        }

        for(const auto& error : errors)
        {
            std::cout << "    " << error << "\n";
        }
    }

    return !bIsError;
}
