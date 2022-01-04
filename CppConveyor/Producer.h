#pragma once

#include "Renderer.h"
#include "Entity.h"

namespace cpp_conv
{
	class Item;

	class Producer : public Entity
	{
	public:
		Producer(int x, int y, Direction direction, Item* pItem, uint64_t productionRate);

		bool IsReadyToProduce() const;

		Item* ProduceItem();

		Direction GetDirection() const { return m_direction; }

		void Tick(cpp_conv::grid::EntityGrid& grid);
		void Draw(HANDLE hConsole, cpp_conv::renderer::ScreenBuffer screenBuffer, cpp_conv::grid::EntityGrid& grid, int x, int y) const;

	private:
		Item* m_pItem;
		Direction m_direction;

		uint64_t m_tick;
		uint64_t m_productionRate;
		bool m_bProductionReady;
	};
}