#include "DataField.h"

bool cpp_conv::TypedDataReader<bool>::Read(const toml::Table* value, const char* szPropertyName, bool& pTargetVariable)
{
    auto [bOk, bValue] = value->getBool(szPropertyName);
    if (!bOk)
    {
        return false;
    }

    pTargetVariable = bValue;
    return true;
}

bool cpp_conv::TypedDataReader<int32_t>::Read(const toml::Table* value, const char* szPropertyName,
                                              int32_t& pTargetVariable)
{
    auto [bOk, iValue] = value->getInt(szPropertyName);
    if (!bOk)
    {
        return false;
    }

    pTargetVariable = static_cast<int32_t>(iValue);
    return true;
}

bool cpp_conv::TypedDataReader<int64_t>::Read(const toml::Table* value, const char* szPropertyName,
                                              int64_t& pTargetVariable)
{
    auto [bOk, iValue] = value->getInt(szPropertyName);
    if (!bOk)
    {
        return false;
    }

    pTargetVariable = iValue;
    return true;
}

bool cpp_conv::TypedDataReader<uint32_t>::Read(const toml::Table* value, const char* szPropertyName,
                                               uint32_t& pTargetVariable)
{
    auto [bOk, iValue] = value->getInt(szPropertyName);
    if (!bOk)
    {
        return false;
    }

    pTargetVariable = static_cast<uint32_t>(iValue);
    return true;
}

bool cpp_conv::TypedDataReader<uint64_t>::Read(const toml::Table* value, const char* szPropertyName,
                                               uint64_t& pTargetVariable)
{
    auto [bOk, iValue] = value->getInt(szPropertyName);
    if (!bOk)
    {
        return false;
    }

    pTargetVariable = static_cast<uint64_t>(iValue);
    return true;
}

bool cpp_conv::TypedDataReader<float>::Read(const toml::Table* value, const char* szPropertyName,
                                            float& pTargetVariable)
{
    auto [bOk, fValue] = value->getDouble(szPropertyName);
    if (!bOk)
    {
        return false;
    }

    pTargetVariable = static_cast<float>(fValue);
    return true;
}

bool cpp_conv::TypedDataReader<std::string>::Read(const toml::Table* value, const char* szPropertyName,
                                                  std::string& pTargetVariable)
{
    auto [bOk, strValue] = value->getString(szPropertyName);
    if (!bOk)
    {
        return false;
    }

    pTargetVariable = strValue;
    return true;
}

bool cpp_conv::TypedDataReader<Eigen::Vector3i>::Read(const toml::Table* value, const char* szPropertyName,
                                              Eigen::Vector3i& pTargetVariable)
{
    const auto vectorTable = value->getTable(szPropertyName);
    if (!vectorTable)
    {
        return false;
    }

    auto [bOkX, x] = vectorTable->getInt("x");
    auto [bOkY, y] = vectorTable->getInt("y");
    auto [bOkZ, z] = vectorTable->getInt("z");
    if (!bOkX || !bOkY || !bOkZ)
    {
        return false;
    }

    pTargetVariable = {static_cast<int32_t>(x), static_cast<int32_t>(y), static_cast<int32_t>(z)};
    return true;
}

bool cpp_conv::TypedDataReader<cpp_conv::resources::registry::RegistryId>::Read(
    const toml::Table* value, const char* szPropertyName, resources::registry::RegistryId& pTargetVariable)
{
    auto [bOk, strValue] = value->getString(szPropertyName);
    if (!bOk)
    {
        return false;
    }

    resources::registry::RegistryId id(resources::registry::RegistryId::Invalid());
    if (!tryLookUpId(strValue, &id))
    {
        return false;
    }

    pTargetVariable = id;
    return true;
}
