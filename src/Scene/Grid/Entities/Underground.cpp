#include "Underground.h"
#include "Conveyor.h"
#include "Sequence.h"
#include "SceneContext.h"

#include "Renderer.h"
#include "RenderContext.h"

#include <algorithm> 
#include <array>
#include <random>
#include <chrono>
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "TileAsset.h"

template <class RandomAccessIterator, class URNG>
void shuffle(RandomAccessIterator first, RandomAccessIterator last, URNG&& g)
{
	for (auto i = (last - first) - 1; i > 0; --i)
	{
		std::uniform_int_distribution<decltype(i)> d(0, i);
		swap(first[i], first[d(g)]);
	}
}

std::tuple<int, Position> GetUndergroundLength(cpp_conv::grid::EntityGrid& grid, cpp_conv::Entity* pStart, Direction direction)
{
	Position kTargetPosition;
	int iTargetUnderground = -1;
	Position kTmpPosition = pStart->m_position;
	for (int i = 0; i < cpp_conv::c_maxUndergroundLength; i++)
	{
		Position kForwardPosition = cpp_conv::grid::GetForwardPosition(kTmpPosition, direction);
		cpp_conv::Entity* pForwardEntity = cpp_conv::grid::SafeGetEntity(grid, kForwardPosition);
		if (pForwardEntity == nullptr || pForwardEntity->m_eEntityKind != EntityKind::Underground)
		{
			kTmpPosition = kForwardPosition;
			continue;
		}

		iTargetUnderground = i + 2; // + 2 for the start and exit points

		// Move forward an extra one to skip the underground block
		kTmpPosition = cpp_conv::grid::GetForwardPosition(kTmpPosition, direction);
		kTargetPosition = cpp_conv::grid::GetForwardPosition(kTmpPosition, direction);
		break;
	}

	return std::make_tuple(iTargetUnderground, kTargetPosition);
}

cpp_conv::Underground::Underground(int x, int y, Direction direction)
	: Entity(x, y, EntityKind::Underground)
	, m_direction(direction)
	, m_arrInternalConveyors
	{
		cpp_conv::Conveyor(x, y, direction, nullptr),
		cpp_conv::Conveyor(x, y, direction, nullptr),
		cpp_conv::Conveyor(x, y, direction, nullptr),
		cpp_conv::Conveyor(x, y, direction, nullptr),
		cpp_conv::Conveyor(x, y, direction, nullptr),
		cpp_conv::Conveyor(x, y, direction, nullptr)
	}	
	, m_uiTick(0)
{
	memset(&m_kLocalGrid, 0, sizeof(m_kLocalGrid));

	Position position = m_position;
	for (int i = 0; i < cpp_conv::c_maxUndergroundLength; i++)
	{
		m_arrInternalConveyors[i].m_position = position;
		position = cpp_conv::grid::GetForwardPosition(position, direction);
	}
}

void cpp_conv::Underground::Tick(const SceneContext& kContext)
{
	int iUndergroundLength;
	Position undergroundEnd;
	
	std::tie(iUndergroundLength, undergroundEnd) = GetUndergroundLength(kContext.m_grid, this, m_direction);
	if (iUndergroundLength == -1)
	{
		return;
	}

	Position position(cpp_conv::c_maxUndergroundLength + 1, cpp_conv::c_maxUndergroundLength + 1);
	
	// TODO[CJones] This isn't really needed.
	for (int i = 0; i < iUndergroundLength; i++)
	{
		m_kLocalGrid[position.m_y][position.m_x] = &m_arrInternalConveyors[i];
		m_arrInternalConveyors[i].m_position = position;

		position = cpp_conv::grid::GetForwardPosition(position, m_direction);
	}

	m_kLocalGrid[position.m_y][position.m_x] = kContext.m_grid[undergroundEnd.m_y][undergroundEnd.m_x];

	SceneContext kLocalContext = { 0, 0, m_kLocalGrid, kContext.m_sequences, kContext.m_conveyors, kContext.m_vOtherEntities };

	Sequence innerSequence(&m_arrInternalConveyors[iUndergroundLength - 1], &m_arrInternalConveyors[0], 0);
	innerSequence.Tick(kLocalContext);

	for (int i = 0; i < iUndergroundLength; i++)
	{
		m_arrInternalConveyors[i].Tick(kLocalContext);
	}

	memset(&m_kLocalGrid, 0, sizeof(m_kLocalGrid));
}

void cpp_conv::Underground::Draw(RenderContext& kRenderContext) const
{
	auto pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Tunnel);
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
			cpp_conv::rotationFromDirection(m_direction)
		},
		{ 0xFF0000FF });
}

bool cpp_conv::Underground::AddItem(cpp_conv::grid::EntityGrid& grid, Item* pItem, int iChannel)
{
	bool bProduced = false;
	Item*& forwardTargetItem = m_arrInternalConveyors[0].m_pChannels[iChannel].m_pItems[0];
	Item*& forwardPendingItem = m_arrInternalConveyors[0].m_pChannels[iChannel].m_pItems[0];
	if (!forwardTargetItem && !forwardPendingItem)
	{
		forwardPendingItem = pItem;
		return true;
	}

	return false;
}
