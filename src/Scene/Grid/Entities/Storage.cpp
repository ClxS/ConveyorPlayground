#include "Storage.h"
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "TileAsset.h"
#include "Profiler.h"

cpp_conv::Storage::Storage(Vector3 position, Vector3 size, uint32_t uiMaxCapacity, uint32_t uiMaxStackSize)
    : Entity(position, size, EntityKind::Storage)
    , m_itemContainer(uiMaxCapacity, uiMaxStackSize, false)
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
            (float)m_position.m_x * cpp_conv::renderer::c_gridScale,
            (float)m_position.m_y * cpp_conv::renderer::c_gridScale,
            cpp_conv::Transform2D::Rotation::DegZero
        },
        { 0xFFFFFF00 });
}

bool cpp_conv::Storage::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, ItemId pItem, int iSourceChannel)
{
    return m_itemContainer.TryInsert(pItem);
}

bool cpp_conv::Storage::TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem)
{
    return m_itemContainer.TryTake(bSingle, outItem);
}
