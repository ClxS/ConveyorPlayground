#pragma once

#include "Entity.h"

namespace cpp_conv
{
	class Item;

	class Producer : public Entity
	{
	public:
		Producer(int x, int y, Direction direction, Item* pItem, uint64_t productionRate)
			: Entity(x, y, EntityKind::Producer)
			, m_pItem(pItem)
			, m_direction(direction)
			, m_tick(0)
			, m_productionRate(productionRate)
			, m_bProductionReady(false)
		{
		}

		void Tick()
		{
			if (m_bProductionReady)
			{
				return;
			}

			m_tick++; 
			if ((m_tick % m_productionRate) == 0)
			{
				m_bProductionReady = true;
			}
		}

		bool IsReadyToProduce() const
		{
			return m_bProductionReady;
		}

		Item* ProduceItem()
		{
			if (!m_bProductionReady)
			{
				return nullptr;
			}

			m_bProductionReady = false;
			return m_pItem;
		}

		Direction GetDirection() const { return m_direction; }

	private:
		Item* m_pItem;
		Direction m_direction;

		uint64_t m_tick;
		uint64_t m_productionRate;
		bool m_bProductionReady;
	};
}