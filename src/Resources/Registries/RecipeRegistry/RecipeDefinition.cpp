#include "RecipeDefinition.h"
#include "AssetPtr.h"
#include "ResourceManager.h"

cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> cpp_conv::RecipeDefinition::GetTile() const
{
    return cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>({ m_registryId.m_index, m_registryId.m_category + 1 });
}
