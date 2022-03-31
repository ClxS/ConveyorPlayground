#include "Conveyor.h"
#include "SceneContext.h"

#include "RenderContext.h"
#include "Renderer.h"
#include "ResourceManager.h"

#include <assert.h>
#include <map>
#include "AssetPtr.h"
#include "DataId.h"
#include "ItemDefinition.h"
#include "ItemInstance.h"
#include "ItemRegistry.h"
#include "Matrix.h"
#include "Profiler.h"
#include "TargetingUtility.h"
#include "TileAsset.h"
#include "TileRenderHandler.h"

cpp_conv::Colour GetColourFromId(int id)
{
    switch (id & 7)
    {
    default:
    case 0: return {0x000000FF};
    case 1: return {0x0000FF00};
    case 2: return {0x0000FFFF};
    case 3: return {0x00FF0000};
    case 4: return {0x00FF00FF};
    case 5: return {0x00FFFF00};
    case 6: return {0x00FFFFFF};
    }
}

void DrawConveyorItem(
    cpp_conv::RenderContext& kContext,
    const cpp_conv::Conveyor& rConveyor,
    cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> pTile,
    Vector2F position,
    uint32_t uiCurrentTick,
    uint32_t uiMoveTick,
    const cpp_conv::ItemInstance& item)
{
    PROFILE_FUNC();

    float fLerpFactor = 1.0f;
    Vector2F previousPosition = {};
    if (item.m_bShouldAnimate && uiMoveTick != 0)
    {
        fLerpFactor = uiCurrentTick / static_cast<float>(uiMoveTick);
        previousPosition = {item.m_PreviousX, item.m_PreviousY};
    }

    if (item.m_bShouldAnimate)
    {
        position = cpp_conv::targeting_util::getRenderPosition(
            kContext.m_rMap,
            rConveyor,
            position,
            fLerpFactor,
            previousPosition);
    }

    tileRenderer(kContext, pTile.get(), {position.GetX(), position.GetY()}, {0xFFFFFFFF}, true);
}

void DrawConveyor(
    cpp_conv::RenderContext& kContext,
    const cpp_conv::Conveyor& rConveyor,
    int x,
    int y,
    bool bIsCorner,
    Direction direction,
    Direction cornerSourceDirection)
{
    const cpp_conv::Transform2D transform = {
        static_cast<float>(x), static_cast<float>(y)
    };
    const auto pTile = rConveyor.GetTile();
    if (pTile)
    {
        tileRenderer(kContext, pTile.get(), transform);
    }
}

cpp_conv::Conveyor::Channel::Channel(int channelLane)
    : m_ChannelLane(channelLane)
{
    std::ranges::fill(m_pSlots, Lane(ItemInstance::Empty(), Vector2F()));
    std::ranges::fill(m_pPendingItems, ItemInstance::Empty());
}

cpp_conv::Conveyor::Conveyor(const Vector3 position, const Vector3 size, const Direction direction, ItemId pItem)
    : Entity(position, size, EntityKind::Conveyor)
      , m_direction(direction)
      , m_pChannels({0, 1})
      , m_uiSequenceIndex(0)
{
}

void cpp_conv::Conveyor::Tick(const SceneContext& kContext)
{
    m_uiCurrentTick++;
    if (m_uiCurrentTick < m_uiMoveTick)
    {
        return;
    }

    m_uiCurrentTick = 0;
    for (int iChannelIdx = 0; iChannelIdx < c_conveyorChannels; iChannelIdx++)
    {
        Channel& rChannel = m_pChannels[iChannelIdx];

        int iChannelLength = c_conveyorChannelSlots;
        if (m_bIsCorner)
        {
            iChannelLength += m_iInnerMostChannel == iChannelIdx ? -1 : 1;
        }

        ItemInstance& rLeadingItem = rChannel.m_pSlots[iChannelLength - 1].m_Item;
        if (!rLeadingItem.IsEmpty())
        {
            Entity* pForwardEntity = kContext.m_rMap.GetEntity(grid::getForwardPosition(*this));
            if (pForwardEntity &&
                pForwardEntity->SupportsInsertion() &&
                pForwardEntity->TryInsert(
                    kContext,
                    *this,
                    InsertInfo(
                        rLeadingItem.m_Item,
                        iChannelIdx,
                        rChannel.m_pSlots[iChannelLength - 1].m_VisualPosition)))
            {
                rLeadingItem = ItemInstance::Empty();
            }
            else
            {
                rLeadingItem.m_bShouldAnimate = false;
            }
        }

        for (int iChannelSlot = rChannel.m_LaneLength - 2; iChannelSlot >= 0; iChannelSlot--)
        {
            ItemInstance& currentItem = rChannel.m_pSlots[iChannelSlot].m_Item;
            ItemInstance& forwardTargetItem = rChannel.m_pSlots[iChannelSlot + 1].m_Item;
            ItemInstance& forwardPendingItem = rChannel.m_pSlots[iChannelSlot + 1].m_Item;
            if (!currentItem.IsEmpty())
            {
                if (forwardTargetItem.IsEmpty() && forwardPendingItem.IsEmpty())
                {
                    PlaceItemInSlot(rChannel.m_ChannelLane, iChannelSlot + 1,
                                    InsertInfo(currentItem.m_Item, iChannelSlot,
                                               rChannel.m_pSlots[iChannelSlot].m_VisualPosition), true);
                    currentItem = ItemInstance::Empty();
                }
                else
                {
                    currentItem.m_bShouldAnimate = false;
                }
            }
        }
    }
}

void cpp_conv::Conveyor::Realize()
{
    for (int iChannelIdx = 0; iChannelIdx < c_conveyorChannels; iChannelIdx++)
    {
        Channel& rChannel = m_pChannels[iChannelIdx];
        int iChannelLength = c_conveyorChannelSlots;
        if (m_bIsCorner)
        {
            iChannelLength += m_iInnerMostChannel == iChannelIdx ? -1 : 1;
        }

        for (int iChannelSlot = 0; iChannelSlot <= iChannelLength - 1; iChannelSlot++)
        {
            if (!rChannel.m_pPendingItems[iChannelSlot].IsEmpty())
            {
                rChannel.m_pSlots[iChannelSlot].m_Item = rChannel.m_pPendingItems[iChannelSlot];
                rChannel.m_pPendingItems[iChannelSlot] = ItemInstance::Empty();
            }
        }
    }
}

void cpp_conv::Conveyor::Draw(RenderContext& kContext) const
{
    PROFILE_FUNC();
    const int conveyorX = m_position.GetX() * 4;
    const int conveyorY = m_position.GetY() * 4;

    const bool bIsCorner = IsCorner();

    switch (kContext.m_uiCurrentDrawPass)
    {
    case 0:
        {
            PROFILE_SCOPE(Conveyor_Pass0)
            DrawConveyor(
                kContext,
                *this,
                conveyorX,
                conveyorY,
                bIsCorner,
                m_direction,
                m_eCornerDirection);
            break;
        }
    case 1:
        {
            PROFILE_SCOPE(Conveyor_Pass1);
            for (int uiChannel = 0; uiChannel < c_conveyorChannels; uiChannel++)
            {
                int iChannelLength = c_conveyorChannelSlots;
                if (m_bIsCorner)
                {
                    iChannelLength += m_iInnerMostChannel == uiChannel ? -1 : 1;
                }

                for (int uiSlot = 0; uiSlot < iChannelLength; uiSlot++)
                {
                    ItemInstance itemInstance;
                    if (!TryPeakItemInSlot(uiChannel, uiSlot, itemInstance))
                    {
                        continue;
                    }

                    const resources::AssetPtr<ItemDefinition> pItem = resources::getItemDefinition(itemInstance.m_Item);
                    if (!pItem)
                    {
                        continue;
                    }

                    uint32_t uiCurrentTick = m_uiCurrentTick;
                    uint32_t uiMoveTick = m_uiMoveTick;
                    if (IsPartOfASequence())
                    {
                        uiCurrentTick = m_pSequence->GetCurrentTick();
                        uiMoveTick = m_pSequence->GetMoveTick();
                    }

                    DrawConveyorItem(
                        kContext,
                        *this,
                        pItem->GetTile(),
                        m_pChannels[uiChannel].m_pSlots[uiSlot].m_VisualPosition,
                        uiCurrentTick,
                        uiMoveTick,
                        itemInstance);
                }
            }

            break;
        }
    default: assert(false);
    }
}

bool cpp_conv::Conveyor::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity,
                                   const InsertInfo insertInfo)
{
    const Channel* pTargetChannel = targeting_util::getTargetChannel(kContext.m_rMap, pSourceEntity, *this,
                                                                     insertInfo.GetSourceChannel());
    if (!pTargetChannel)
    {
        return false;
    }

    const int forwardTargetItemSlot = targeting_util::getChannelTargetSlot(
        kContext.m_rMap, pSourceEntity, *this, insertInfo.GetSourceChannel());
    if (HasItemInSlot(pTargetChannel->m_ChannelLane, forwardTargetItemSlot))
    {
        return false;
    }

    PlaceItemInSlot(pTargetChannel->m_ChannelLane, forwardTargetItemSlot, insertInfo);
    return true;
}

bool cpp_conv::Conveyor::TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem)
{
    if (IsPartOfASequence())
    {
        for (int iChannelLane(0); iChannelLane < 2; ++iChannelLane)
        {
            for (int iChannelSlot(0); iChannelSlot < 2; ++iChannelSlot)
            {
                if (!m_pSequence->HasItemInSlot(m_uiSequenceIndex, iChannelLane, iChannelSlot))
                {
                    continue;
                }

                ItemId item;
                Vector2F origin;
                if (m_pSequence->RemoveItemFromSlot(m_uiSequenceIndex, iChannelLane, iChannelSlot, item, origin))
                {
                    outItem = std::make_tuple(item, 1);
                    return true;
                }
            }
        }
    }
    else
    {
        const bool bIsCorner = IsCorner();
        auto [iInnerMostChannel, eCornerDirection] = getInnerMostCornerChannel(kContext.m_rMap, *this);

        for (int iChannelIdx = 0; iChannelIdx < c_conveyorChannels; ++iChannelIdx)
        {
            int iChannelLength = c_conveyorChannelSlots;
            if (bIsCorner)
            {
                iChannelLength += iInnerMostChannel == iChannelIdx ? -1 : 1;
            }

            for (int iChannelSlot = 0; iChannelSlot < iChannelLength; ++iChannelSlot)
            {
                if (!m_pChannels[iChannelIdx].m_pSlots[iChannelSlot].m_Item.IsEmpty())
                {
                    outItem = std::make_tuple(m_pChannels[iChannelIdx].m_pSlots[iChannelSlot].m_Item.m_Item, 1);
                    m_pChannels[iChannelIdx].m_pSlots[iChannelSlot].m_Item = ItemInstance::Empty();
                    return true;
                }
            }
        }
    }

    return false;
}

std::string cpp_conv::Conveyor::GetDescription() const
{
    std::map<ItemId, int> storedItems;

    for (int iChannelIdx = 0; iChannelIdx < c_conveyorChannels; ++iChannelIdx)
    {
        for (int iChannelSlot = 0; iChannelSlot < c_conveyorChannelSlots; ++iChannelSlot)
        {
            ItemId item = m_pChannels[iChannelIdx].m_pSlots[iChannelSlot].m_Item.m_Item;
            if (!item.IsEmpty())
            {
                storedItems.try_emplace(item, 0);
                storedItems[item]++;
            }
        }
    }

    std::string str;
    bool bFirst = true;
    if (storedItems.empty())
    {
        str = "Conveyor is empty!";
    }
    else
    {
        for (auto& [item, count] : storedItems)
        {
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                str += ", ";
            }

            resources::AssetPtr<ItemDefinition> pItem = resources::getItemDefinition(item);
            if (pItem)
            {
                str += std::format("{} {}", count, pItem->GetName());
            }
            else
            {
                str += std::format("{} Unknown Items", count);
            }
        }
    }

    return str;
}

void cpp_conv::Conveyor::OnLocalityUpdate(const WorldMap& map)
{
    const Entity* pEntity = map.GetEntity(grid::getForwardPosition(m_position, m_direction));

    m_bIsCorner = targeting_util::isCornerConveyor(map, *this);
    m_bIsClockwise = targeting_util::isClockwiseCorner(map, *this);
    m_bIsCapped = !pEntity || !pEntity->SupportsInsertion();

    std::tie(m_iInnerMostChannel, m_eCornerDirection) = getInnerMostCornerChannel(map, *this);

    for (int iLane = 0; iLane < static_cast<int>(m_pChannels.size()); iLane++)
    {
        m_pChannels[iLane].m_LaneLength = 2;
        if (m_bIsCorner)
        {
            m_pChannels[iLane].m_LaneLength += GetInnerMostChannel() == iLane ? -1 : 1;
        }

        for (int iSlot = 0; iSlot < static_cast<int>(m_pChannels[iLane].m_pSlots.size()); iSlot++)
        {
            m_pChannels[iLane].m_pSlots[iSlot].m_VisualPosition = targeting_util::getRenderPosition(
                map, *this, {iLane, iSlot});
        }
    }

    if (!m_bIsCorner)
    {
        if (!IsCapped())
        {
            m_pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(
                resources::registry::assets::conveyors::c_ConveyorStraight);
        }
        else
        {
            m_pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(
                resources::registry::assets::conveyors::c_ConveyorStraightCappedFront);
        }
    }
    else
    {
        if (IsClockwiseCorner())
        {
            m_pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(
                resources::registry::assets::conveyors::c_ConveyorCornerClockwise);
        }
        else
        {
            m_pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(
                resources::registry::assets::conveyors::c_ConveyorCornerAntiClockwise);
        }
    }
}

void cpp_conv::Conveyor::SetSequence(Sequence* pSequence, uint8_t sequenceIndex)
{
    m_pSequence = pSequence;
    m_uiSequenceIndex = sequenceIndex;
}

void cpp_conv::Conveyor::ClearSequence()
{
    m_pSequence = nullptr;
    m_uiSequenceIndex = 0;
}

uint64_t cpp_conv::Conveyor::CountItemsOnBelt()
{
    uint64_t uiCount = 0;
    for (auto& pChannel : m_pChannels)
    {
        for (auto& [item, _] : pChannel.m_pSlots)
        {
            if (!item.IsEmpty())
            {
                uiCount++;
            }
        }
    }

    return uiCount;
}

bool cpp_conv::Conveyor::HasItemInSlot(int lane, int slot) const
{
    if (IsPartOfASequence())
    {
        return m_pSequence->HasItemInSlot(m_uiSequenceIndex, lane, slot);
    }

    const Channel& rTargetChannel = m_pChannels[lane];
    const ItemInstance& forwardTargetItem = rTargetChannel.m_pSlots[slot].m_Item;
    const ItemInstance& forwardPendingItem = rTargetChannel.m_pPendingItems[slot];

    return (!forwardTargetItem.IsEmpty() || !forwardPendingItem.IsEmpty());
}

void cpp_conv::Conveyor::PlaceItemInSlot(const int lane, const int slot, const InsertInfo insertInfo,
                                         const bool bDirectItemSet)
{
    assert(!HasItemInSlot(lane, slot));

    if (IsPartOfASequence())
    {
        if (insertInfo.HasOriginPosition())
        {
            const Vector2F position = insertInfo.GetOriginPosition();
            m_pSequence->AddItemInSlot(m_uiSequenceIndex, lane, slot, insertInfo.GetItem(), &position);
        }
        else
        {
            m_pSequence->AddItemInSlot(m_uiSequenceIndex, lane, slot, insertInfo.GetItem(), nullptr);
        }
        return;
    }

    ItemInstance& forwardTargetItem = (bDirectItemSet
                                           ? m_pChannels[lane].m_pSlots[slot].m_Item
                                           : m_pChannels[lane].m_pPendingItems[slot]);
    if (m_eEntityKind != EntityKind::Conveyor)
    {
        forwardTargetItem = {insertInfo.GetItem(), 0, 0, false};
        return;
    }

    if (insertInfo.HasOriginPosition())
    {
        forwardTargetItem = {
            insertInfo.GetItem(), insertInfo.GetOriginPosition().GetX(), insertInfo.GetOriginPosition().GetY(), true
        };
    }
    else
    {
        forwardTargetItem = {insertInfo.GetItem(), 0, 0, false};
    }
}

bool cpp_conv::Conveyor::TryPeakItemInSlot(const int lane, const int slot, ItemInstance& rItem) const
{
    if (IsPartOfASequence())
    {
        return m_pSequence->TryPeakItemInSlot(m_uiSequenceIndex, lane, slot, rItem);
    }

    if (m_pChannels[lane].m_pSlots[slot].m_Item.IsEmpty())
    {
        return false;
    }

    rItem = m_pChannels[lane].m_pSlots[slot].m_Item;
    return true;
}
