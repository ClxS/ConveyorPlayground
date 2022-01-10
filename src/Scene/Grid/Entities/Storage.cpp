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

bool cpp_conv::Storage::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, ItemId pItem, int iSourceChannel)
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

bool cpp_conv::Storage::TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem)
{
    if (m_vItemEntries.empty())
    {
        return false;
    }

    uint32_t uiCount = 1;
    if (bSingle)
    {
        uiCount = m_vItemEntries[0].m_pCount;
    }

    outItem = std::make_tuple(m_vItemEntries[0].m_pItem, m_vItemEntries[0].m_pCount);

    m_vItemEntries[0].m_pCount -= uiCount;
    if (m_vItemEntries[0].m_pCount == 0)
    {
        m_vItemEntries.erase(m_vItemEntries.begin());
    }

    return true;
}
