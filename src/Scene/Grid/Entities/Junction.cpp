#include "Junction.h"
#include "Conveyor.h"
#include "SceneContext.h"

#include "Renderer.h"
#include "RenderContext.h"

#include <algorithm> 
#include <array>
#include <random>
#include <chrono>
#include "TileAsset.h"
#include "ResourceManager.h"

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
    , m_pItem(cpp_conv::ItemIds::None)
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
    std::array<std::tuple<cpp_conv::Entity*, Direction>, 4> arrDirectionEntities =
    {
        std::make_tuple(kContext.m_rMap.GetEntity(cpp_conv::grid::GetForwardPosition(m_position, Direction::Up)), Direction::Up),
        std::make_tuple(kContext.m_rMap.GetEntity(cpp_conv::grid::GetForwardPosition(m_position, Direction::Right)), Direction::Right),
        std::make_tuple(kContext.m_rMap.GetEntity(cpp_conv::grid::GetForwardPosition(m_position, Direction::Down)), Direction::Down),
        std::make_tuple(kContext.m_rMap.GetEntity(cpp_conv::grid::GetForwardPosition(m_position, Direction::Left)), Direction::Left),
    };

    std::default_random_engine engine(m_uiTick % 256);
    for (auto i = (arrDirectionEntities.end() - arrDirectionEntities.begin()) - 1; i > 0; --i)
    {
        std::uniform_int_distribution<decltype(i)> d(0, i);
        std::swap(arrDirectionEntities.begin()[i], arrDirectionEntities.begin()[d(engine)]);
    }

    for (auto entityDirectionPair : arrDirectionEntities)
    {
        cpp_conv::Entity* pEntity;
        Direction direction;

        std::tie(pEntity, direction) = entityDirectionPair;

        if (pEntity == nullptr || !pEntity->SupportsInsertion())
        {
            continue;  
        }

        bool bFound = false;
        for (int iExitChannel = 0; iExitChannel < cpp_conv::c_conveyorChannels; ++iExitChannel)
        {
            if (pEntity->TryInsert(kContext, *this, m_pItem, iExitChannel))
            {
                m_pItem = cpp_conv::ItemIds::None;
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
    auto pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Junction);
    if (!pTile)
    {
        return;
    }

    cpp_conv::renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            (float)m_position.GetX() * cpp_conv::renderer::c_gridScale,
            (float)m_position.GetY() * cpp_conv::renderer::c_gridScale,
            Rotation::DegZero
        },
        { 0xFFFF00FF });
}

bool cpp_conv::Junction::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, ItemId pItem, int iSourceChannel, int iSourceLane)
{
    if (!m_pItem.IsEmpty())
    {
        return false;
    }

    m_pItem = pItem;
    return true;
}
