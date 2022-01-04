#include "Producer.h"

cpp_conv::Producer::Producer(int x, int y, Direction direction, Item* pItem, uint64_t productionRate)
	: Entity(x, y, EntityKind::Producer)
	, m_pItem(pItem)
	, m_direction(direction)
	, m_tick(0)
	, m_productionRate(productionRate)
	, m_bProductionReady(false)
{
}

void cpp_conv::Producer::Tick()
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

bool cpp_conv::Producer::IsReadyToProduce() const
{
	return m_bProductionReady;
}

cpp_conv::Item* cpp_conv::Producer::ProduceItem()
{
	if (!m_bProductionReady)
	{
		return nullptr;
	}

	m_bProductionReady = false;
	return m_pItem;
}

void cpp_conv::Producer::Draw(HANDLE hConsole, cpp_conv::renderer::ScreenBuffer screenBuffer, cpp_conv::grid::EntityGrid& grid, int x, int y) const
{
	for (int conveyorY = y * 3 + 1; conveyorY < y * 3 + 3; conveyorY++)
	{
		for (int conveyorX = x * 3 + 1; conveyorX < x * 3 + 3; conveyorX++)
		{
			switch (m_direction)
			{
			case Direction::Left:
				cpp_conv::renderer::setPixel(hConsole, screenBuffer, L'←', conveyorX, conveyorY, 0, true);
				break;
			case Direction::Up:
				cpp_conv::renderer::setPixel(hConsole, screenBuffer, L'↓', conveyorX, conveyorY, 0, true);
				break;
			case Direction::Right:
				cpp_conv::renderer::setPixel(hConsole, screenBuffer, L'→', conveyorX, conveyorY, 0, true);
				break;
			case Direction::Down:
				cpp_conv::renderer::setPixel(hConsole, screenBuffer, L'↑', conveyorX, conveyorY, 0, true);
				break;
			}
		}
	}
}
