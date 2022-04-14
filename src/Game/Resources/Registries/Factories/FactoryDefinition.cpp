#include "FactoryDefinition.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/ResourceLoader.h"

atlas::resource::AssetPtr<cpp_conv::resources::TileAsset> cpp_conv::FactoryDefinition::GetTile() const
{
    if (!m_AssetId.m_Value.IsValid())
    {
        return nullptr;
    }

    return atlas::resource::ResourceLoader::LoadAsset<resources::TileAsset>(m_AssetId.m_Value);
}
