#include "Tunnel.h"
#include "Conveyor.h"
#include "SceneContext.h"

#include "RenderContext.h"
#include "Renderer.h"

#include <array>
#include <random>
#include "Direction.h"
#include "ResourceManager.h"
#include "TargetingUtility.h"
#include "TileAsset.h"
#include "WorldMap.h"

template <class RandomAccessIterator, class URNG>
void shuffle(RandomAccessIterator first, RandomAccessIterator last, URNG&& g)
{
    for (auto i = (last - first) - 1; i > 0; --i)
    {
        std::uniform_int_distribution<decltype(i)> d(0, i);
        swap(first[i], first[d(g)]);
    }
}

std::tuple<int, Vector3> GetUndergroundLength(const cpp_conv::WorldMap& map, const cpp_conv::Entity* pStart, Direction direction)
{
    Vector3 kTargetPosition;
    int iTargetUnderground = -1;
    Vector3 kTmpPosition = pStart->m_position;
    for (int i = 0; i < cpp_conv::c_maxUndergroundLength; i++)
    {
        const Vector3 kForwardPosition = cpp_conv::grid::getForwardPosition(kTmpPosition, direction);
        const cpp_conv::Entity* pForwardEntity = map.GetEntity(kForwardPosition);
        if (pForwardEntity == nullptr || pForwardEntity->m_eEntityKind != EntityKind::Tunnel)
        {
            kTmpPosition = kForwardPosition;
            continue;
        }

        iTargetUnderground = i + 1; // + 1 for the start point

        // Move forward an extra one to skip the underground block
        kTmpPosition = cpp_conv::grid::getForwardPosition(kTmpPosition, direction);
        kTargetPosition = cpp_conv::grid::getForwardPosition(kTmpPosition, direction);
        break;
    }

    return std::make_tuple(iTargetUnderground, kTargetPosition);
}

cpp_conv::Tunnel::Tunnel(const Vector3 position, const Vector3 size, const Direction direction)
    : Entity(position, size, EntityKind::Tunnel)
    , m_pOtherSide{nullptr}
    , m_pSequence{nullptr}
    , m_Direction(direction)
{
}

void cpp_conv::Tunnel::Tick(const SceneContext& kContext)
{
    if (m_pSequence == nullptr || m_pSequence->GetHeadEntity() != this)
    {
        return;
    }

    m_pSequence->Tick(kContext);
}

void cpp_conv::Tunnel::Realize()
{
    if (m_pSequence == nullptr || m_pSequence->GetHeadEntity() != this)
    {
        return;
    }

    m_pSequence->Realize();
}

void cpp_conv::Tunnel::Draw(RenderContext& kRenderContext) const
{
    const auto pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(resources::registry::assets::conveyors::c_Tunnel);
    if (!pTile)
    {
        return;
    }

    auto [iUndergroundLength, undergroundEnd] = GetUndergroundLength(kRenderContext.m_rMap, this, m_Direction);

    Rotation eDirection = rotationFromDirection(m_Direction);
    if (iUndergroundLength == -1)
    {
        eDirection = rotationFromDirection(
            direction::rotate90DegreeClockwise(
                direction::rotate90DegreeClockwise(m_Direction)));
    }

    renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            static_cast<float>(m_position.GetX()) * renderer::c_gridScale,
            static_cast<float>(m_position.GetY()) * renderer::c_gridScale,
            eDirection
        },
        { 0xFF0000FF });
}

bool cpp_conv::Tunnel::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const InsertInfo insertInfo)
{
    if (!m_pSequence)
    {
        return false;
    }

    if (m_pSequence->HasItemInSlot(0, insertInfo.GetSourceChannel(), 0))
    {
        return false;
    }

    const Vector2F position = insertInfo.GetOriginPosition();
    m_pSequence->AddItemInSlot(0, insertInfo.GetSourceChannel(), 0, insertInfo.GetItem(), &position);
    return true;
}

void cpp_conv::Tunnel::OnLocalityUpdate(const WorldMap& map)
{
    Vector3 position = m_position;
    if (m_pOtherSide != nullptr)
    {
        return;
    }

    for (uint32_t uiDistance = 0; uiDistance < c_uiMaxLength; ++uiDistance)
    {
        position = grid::getBackwardsPosition(position, GetDirection());

        Entity* pEntity = map.GetEntity(position);
        if (!pEntity || pEntity->m_eEntityKind != EntityKind::Tunnel || pEntity->GetDirection() != GetDirection())
        {
            continue;
        }

        const auto pOtherTunnel = static_cast<Tunnel*>(pEntity);
        if (pOtherTunnel->m_pOtherSide == this)
        {
            break;
        }

        if (pOtherTunnel->m_pOtherSide != nullptr)
        {
            continue;
        }

        m_pSequence = new Sequence(this, uiDistance + 2,
            targeting_util::getTilePosition(map, *pOtherTunnel, { 0, 0 }),
            targeting_util::getTilePosition(map, *pOtherTunnel, { 1, 0 }),
            targeting_util::getTilePosition(map, *pOtherTunnel, { 0, 1 }) - targeting_util::getTilePosition(map, *pOtherTunnel, { 0, 0 }),
            m_uiMoveTick);
        pOtherTunnel->m_pSequence = m_pSequence;

        pOtherTunnel->m_pOtherSide = this;
        m_pOtherSide = pOtherTunnel;
        break;
    }
}
