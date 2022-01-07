#include "Storage.h"
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "TileAsset.h"

cpp_conv::Storage::Storage(int x, int y) : Entity(x, y, EntityKind::Storage)
{
}

void cpp_conv::Storage::Tick(const SceneContext& kContext)
{

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
