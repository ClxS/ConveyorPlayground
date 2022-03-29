#include "Junction.h"
#include "Conveyor.h"
#include "SceneContext.h"

#include "RenderContext.h"
#include "Renderer.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <random>
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

cpp_conv::Junction::Junction(Vector3 position, Vector3 size)
    : Entity(position, size, EntityKind::Junction)
      , m_pItem(ItemIds::None)
      , m_uiTick(0)
{
}

void cpp_conv::Junction::Tick(const SceneContext& kContext)
{
    if (!m_pItem.IsValid())
    {
        return;
    }

    m_uiTick++;
    std::array<std::tuple<Entity*, Direction>, 4> arrDirectionEntities =
    {
        std::make_tuple(kContext.m_rMap.GetEntity(grid::getForwardPosition(m_position, Direction::Up)), Direction::Up),
        std::make_tuple(kContext.m_rMap.GetEntity(grid::getForwardPosition(m_position, Direction::Right)),
                        Direction::Right),
        std::make_tuple(kContext.m_rMap.GetEntity(grid::getForwardPosition(m_position, Direction::Down)),
                        Direction::Down),
        std::make_tuple(kContext.m_rMap.GetEntity(grid::getForwardPosition(m_position, Direction::Left)),
                        Direction::Left),
    };

    std::default_random_engine engine(m_uiTick % 256);
    for (auto i = (arrDirectionEntities.end() - arrDirectionEntities.begin()) - 1; i > 0; --i)
    {
        std::uniform_int_distribution<decltype(i)> d(0, i);
        std::swap(arrDirectionEntities.begin()[i], arrDirectionEntities.begin()[d(engine)]);
    }

    for (const auto entityDirectionPair : arrDirectionEntities)
    {
        Entity* pEntity;
        Direction direction;

        std::tie(pEntity, direction) = entityDirectionPair;

        if (pEntity == nullptr || !pEntity->SupportsInsertion())
        {
            continue;
        }

        if (pEntity->m_eEntityKind == EntityKind::Conveyor)
        {
            const Conveyor* pConveyor = static_cast<Conveyor*>(pEntity);

            // Check it's not an input conveyor
            if (grid::getForwardPosition(pConveyor->m_position, pConveyor->m_direction) == m_position)
            {
                continue;
            }
        }

        bool bFound = false;
        for (int iExitChannel = 0; iExitChannel < c_conveyorChannels; ++iExitChannel)
        {
            if (pEntity->TryInsert(kContext, *this, InsertInfo(m_pItem, iExitChannel)))
            {
                m_pItem = ItemIds::None;
                bFound = true;
                break;
            }
        }

        if (bFound)
        {
            break;
        }
    }
}

void cpp_conv::Junction::Draw(RenderContext& kRenderContext) const
{
    const auto pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(
        resources::registry::assets::conveyors::c_Junction);
    if (!pTile)
    {
        return;
    }

    renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            static_cast<float>(m_position.GetX()) * renderer::c_gridScale,
            static_cast<float>(m_position.GetY()) * renderer::c_gridScale,
            Rotation::DegZero
        },
        {0xFFFF00FF});
}

bool cpp_conv::Junction::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity,
                                   const InsertInfo insertInfo)
{
    if (!m_pItem.IsEmpty())
    {
        return false;
    }

    m_pItem = insertInfo.GetItem();
    return true;
}
