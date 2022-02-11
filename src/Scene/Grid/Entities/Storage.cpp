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
    const auto pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(resources::registry::visual::Storage);
    if (!pTile)
    {
        return;
    }

    renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            (float)m_position.GetX() * renderer::c_gridScale,
            (float)m_position.GetY() * renderer::c_gridScale,
            Rotation::DegZero
        },
        { 0xFFFFFF00 });
}

bool cpp_conv::Storage::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const InsertInfo insertInfo)
{
    return m_itemContainer.TryInsert(insertInfo.GetItem());
}

bool cpp_conv::Storage::TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem)
{
    return m_itemContainer.TryTake(bSingle, outItem);
}
