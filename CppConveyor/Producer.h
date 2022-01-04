#pragma once

#include "Entity.h"

namespace cpp_conv
{
	class Item;

	class Producer : public Entity
	{
	public:
		Producer(int x, int y, Direction direction, Item* pItem)
			: Entity(x, y, EntityKind::Producer)
			, m_pItem(pItem)
			, m_direction(direction)
		{
		}

		Item* GetItem() const { return m_pItem; }

		Direction GetDirection() const { return m_direction; }

	private:
		Item* m_pItem;
		Direction m_direction;
	};
}