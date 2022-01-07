#include "Storage.h"
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "TileAsset.h"
#include "Profiler.h"

cpp_conv::Storage::Storage(int x, int y, uint32_t uiMaxCapacity, uint32_t uiMaxStackSize)
    : Entity(x, y, EntityKind::Storage)
    , m_uiMaxCapacity(uiMaxCapacity)
    , m_uiMaxStackSize(uiMaxStackSize)
{
}

void cpp_conv::Storage::Tick(const SceneContext& kContext)
{
    PROFILE_FUNC();
}

void cpp_conv::Storage::Draw(RenderContext& kRenderContext) const
{
    auto pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Storage);
    if (!pTile)
    {
        return;
    }

    cpp_conv::renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            m_position.m_x * cpp_conv::renderer::c_gridScale,
            m_position.m_y * cpp_conv::renderer::c_gridScale,
            cpp_conv::Transform2D::Rotation::DegZero
        },
        { 0xFFFFFF00 });
}

bool cpp_conv::Storage::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const Item* pItem, int iSourceChannel)
{
    PROFILE_FUNC();
    for (auto& rItemEntry : m_vItemEntries)
    {
        if (rItemEntry.m_pItem == pItem && rItemEntry.m_pCount < m_uiMaxStackSize)
        {
            rItemEntry.m_pCount++;
            return true;
        }
    }

    if (m_vItemEntries.size() < m_uiMaxCapacity)
    {
        m_vItemEntries.emplace_back(pItem, 1);
    }

    return false;
}
