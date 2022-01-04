#pragma once

#include "Renderer.h"
#include "Entity.h"

namespace cpp_conv
{
	class Item;

	class Junction : public Entity
	{
	public:
		Junction(int x, int y);

		void Tick(cpp_conv::grid::EntityGrid& grid);
		void Draw(HANDLE hConsole, cpp_conv::renderer::ScreenBuffer screenBuffer, cpp_conv::grid::EntityGrid& grid, int x, int y) const;

		bool HasSpace() const;

		void AddItem(Item* pItem);

	private:
		Item* m_pItem;
		uint64_t m_uiTick;
	};
}