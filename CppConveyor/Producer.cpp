#include "Producer.h"
#include "Conveyor.h"

cpp_conv::Producer::Producer(int x, int y, Direction direction, Item* pItem, uint64_t productionRate)
	: Entity(x, y, EntityKind::Producer)
	, m_pItem(pItem)
	, m_direction(direction)
	, m_tick(0)
	, m_productionRate(productionRate)
	, m_bProductionReady(false)
{
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

void cpp_conv::Producer::Tick(cpp_conv::grid::EntityGrid& grid)
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

	if (!IsReadyToProduce())
	{
		return;
	}

	Item* pItem = ProduceItem();
	if (!pItem)
	{
		return;
	}

	cpp_conv::Entity* pForwardEntity = cpp_conv::grid::SafeGetEntity(grid, cpp_conv::grid::GetForwardPosition(*this, GetDirection()));
	if (!pForwardEntity || pForwardEntity->m_eEntityKind != EntityKind::Conveyor)
	{
		return;
	}

	cpp_conv::Conveyor* pConveyor = reinterpret_cast<cpp_conv::Conveyor*>(pForwardEntity);
	Item*& forwardTargetItem = pConveyor->m_pChannels[rand() % cpp_conv::c_conveyorChannels].m_pItems[0];
	Item*& forwardPendingItem = pConveyor->m_pChannels[rand() % cpp_conv::c_conveyorChannels].m_pItems[0];
	if (!forwardTargetItem && !forwardPendingItem)
	{
		forwardPendingItem = pItem;
	}
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
