#pragma once

#include "Entity.h"
#include "Conveyor.h"
#include "Grid.h"

namespace cpp_conv
{
	class Item;
	struct SceneContext;
	struct RenderContext;

	constexpr int c_maxUndergroundLength = 6;

	class Underground : public Entity
	{
	public:
		Underground(int x, int y, Direction direction);

		void Tick(const SceneContext& kContext);
		void Draw(RenderContext& kContext) const;

		bool AddItem(cpp_conv::grid::EntityGrid& grid, Item* pItem, int iChannel);

	private:
		cpp_conv::grid::EntityGrid m_kLocalGrid;
		Direction m_direction;
		std::array<Conveyor, c_maxUndergroundLength> m_arrInternalConveyors;
		uint64_t m_uiTick;
	};
}