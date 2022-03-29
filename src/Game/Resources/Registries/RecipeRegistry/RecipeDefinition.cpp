#include "RecipeDefinition.h"
#include "AssetPtr.h"
#include "ResourceManager.h"

bool cpp_conv::TypedDataReader<cpp_conv::RecipeDefinition::RecipeItem>::Read(const toml::Table* value,
                                                                             const char* szPropertyName,
                                                                             RecipeDefinition::RecipeItem&
                                                                             pTargetVariable)
{
    uint32_t iValue;
    const bool bResult = TypedDataReader<uint32_t>::Read(value, szPropertyName, iValue);
    if (!bResult)
    {
        return false;
    }

    pTargetVariable.m_idItem = ItemId::FromStringId(szPropertyName);
    pTargetVariable.m_uiCount = iValue;
    return true;
}

bool cpp_conv::TypedDataReader<std::vector<cpp_conv::RecipeDefinition::RecipeItem>>::Read(const toml::Table* value,
    const char* szPropertyName, std::vector<RecipeDefinition::RecipeItem>& pTargetVariable)
{
    std::string strValue;
    const auto recipeTable = value->getTable(szPropertyName);
    if (!recipeTable)
    {
        return false;
    }

    for (auto& key : recipeTable->keys())
    {
        RecipeDefinition::RecipeItem item;
        const bool bResult = TypedDataReader<RecipeDefinition::RecipeItem>::Read(recipeTable.get(), key.c_str(), item);
        if (!bResult)
        {
            return false;
        }

        pTargetVariable.push_back(item);
    }

    return true;
}
