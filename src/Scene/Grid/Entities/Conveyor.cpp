#include "Conveyor.h"
#include "SceneContext.h"

#include "Renderer.h"
#include "RenderContext.h"
#include "ResourceManager.h"

#include "TileAsset.h"
#include "Profiler.h"
#include "TargetingUtility.h"
#include "ItemRegistry.h"
#include "ItemDefinition.h"
#include "AssetPtr.h"
#include <map>
#include "DataId.h"
#include "Matrix.h"
#include "ItemInstance.h"
#include "TileRenderHandler.h"

cpp_conv::Colour GetColourFromId(int id)
{
    switch (id & 7)
    {
    default:
    case 0: return { 0x000000FF };
    case 1: return { 0x0000FF00 };
    case 2: return { 0x0000FFFF };
    case 3: return { 0x00FF0000 };
    case 4: return { 0x00FF00FF };
    case 5: return { 0x00FFFF00 };
    case 6: return { 0x00FFFFFF };
    }
}

void DrawConveyorItem(
    cpp_conv::RenderContext& kContext,
    const cpp_conv::Conveyor& rConveyor,
    cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> pTile,
    Vector2F position,
    const cpp_conv::ItemInstance& item)
{
    PROFILE_FUNC();

    float fLerpFactor = 1.0f;
    Vector2F previousPosition = {};
    if (item.m_bShouldAnimate && item.m_TargetTick != 0)
    {
        fLerpFactor = item.m_CurrentTick / (float)item.m_TargetTick;
        previousPosition = { item.m_PreviousX, item.m_PreviousY };
    }
     
    if (item.m_bShouldAnimate)
    {
        position = cpp_conv::targeting_util::GetRenderPosition(
            kContext.m_rMap,
            rConveyor,
            position,
            fLerpFactor,
            previousPosition);
    }

    tileRenderer(kContext, pTile.get(), { position.GetX(), position.GetY() });
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
    cpp_conv::Transform2D transform = { (float)x, (float)y, cpp_conv::rotationFromDirection(direction) };
    auto pTile = rConveyor.GetTile();
    if (pTile)
    {
        tileRenderer(kContext, pTile.get(), transform);
    }
}

cpp_conv::Conveyor::Channel::Channel(int channelLane)
    : m_ChannelLane(channelLane)
{
    std::fill(std::begin(m_pSlots), std::end(m_pSlots), Lane(ItemInstance::Empty(), Vector2F()));
    std::fill(std::begin(m_pPendingItems), std::end(m_pPendingItems), ItemInstance::Empty());
}

cpp_conv::Conveyor::Conveyor(Vector3 position, Vector3 size, Direction direction, ItemId pItem)
    : Entity(position, size, EntityKind::Conveyor)
    , m_direction(direction)
    , m_pSequenceId(0)
    , m_pChannels({ 0, 1 })
{
}

void cpp_conv::Conveyor::Tick(const SceneContext& kContext)
{
    m_bHasWork = false;

    bool bIsCornerConveyor = IsCorner();
    for (int iChannelIdx = 0; iChannelIdx < cpp_conv::c_conveyorChannels; iChannelIdx++)
    {
        cpp_conv::Conveyor::Channel& rChannel = m_pChannels[iChannelIdx];

        int iChannelLength = cpp_conv::c_conveyorChannelSlots;
        if (bIsCornerConveyor)
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

            if (!rChannel.m_pSlots[iChannelSlot].m_Item.IsEmpty())
            {
                ItemInstance& rItem = rChannel.m_pSlots[iChannelSlot].m_Item;
                if (rItem.m_CurrentTick < rChannel.m_pSlots[iChannelSlot].m_Item.m_TargetTick)
                {
                    ++rItem.m_CurrentTick;
                }

                if (rItem.m_CurrentTick >= rChannel.m_pSlots[iChannelSlot].m_Item.m_TargetTick)
                {
                    m_bHasWork = true;
                }
            }
        }
    }
}

void cpp_conv::Conveyor::Draw(RenderContext& kContext) const
{
    PROFILE_FUNC();
    int conveyorX = m_position.GetX() * 4;
    int conveyorY = m_position.GetY() * 4;

    bool bIsCorner = IsCorner();

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
        for (auto& rChannel : m_pChannels)
        {
            for (auto& rSlot : rChannel.m_pSlots)
            {
                if (rSlot.m_Item.IsEmpty())
                {
                    continue;
                }

                cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition> pItem = cpp_conv::resources::getItemDefinition(rSlot.m_Item.m_Item);
                if (!pItem)
                {
                    continue;
                }

                DrawConveyorItem(
                    kContext,
                    *this,
                    pItem->GetTile(),
                    rSlot.m_VisualPosition,
                    rSlot.m_Item);
            }
        }
        break;
    }
    }
}

bool cpp_conv::Conveyor::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const ItemId pItem, int iSourceChannel, int iSourceLane)
{
    cpp_conv::Conveyor::Channel* pTargetChannel = cpp_conv::targeting_util::GetTargetChannel(kContext.m_rMap, pSourceEntity, *this, iSourceChannel);
    if (!pTargetChannel)
    {
        return false;
    }

    int forwardTargetItemSlot = cpp_conv::targeting_util::GetChannelTargetSlot(kContext.m_rMap, pSourceEntity, *this, iSourceChannel);

    ItemInstance& forwardTargetItem = pTargetChannel->m_pSlots[forwardTargetItemSlot].m_Item;
    ItemInstance& forwardPendingItem = pTargetChannel->m_pPendingItems[forwardTargetItemSlot];

    // Following node is empty, we can just move there
    if (!forwardTargetItem.IsEmpty() || !forwardPendingItem.IsEmpty())
    {
        return false;
    }

    AddItemToSlot(kContext.m_rMap, pTargetChannel, forwardTargetItemSlot, pItem, pSourceEntity, iSourceChannel, iSourceLane);
    return true;
}

bool cpp_conv::Conveyor::TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem)
{
    bool bIsCorner = IsCorner();
    int iInnerMostChannel;
    Direction eCornerDirection;
    std::tie(iInnerMostChannel, eCornerDirection) = GetInnerMostCornerChannel(kContext.m_rMap, *this);

    for (int iChannelIdx = 0; iChannelIdx < c_conveyorChannels; ++iChannelIdx)
    {
        int iChannelLength = cpp_conv::c_conveyorChannelSlots;
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

    return false;
}

std::string cpp_conv::Conveyor::GetDescription() const
{
    std::map<cpp_conv::ItemId, int> storedItems;

    for (int iChannelIdx = 0; iChannelIdx < c_conveyorChannels; ++iChannelIdx)
    {
        for (int iChannelSlot = 0; iChannelSlot < cpp_conv::c_conveyorChannelSlots; ++iChannelSlot)
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
        for (auto& item : storedItems)
        {
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                str += ", ";
            }

            cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition> pItem = cpp_conv::resources::getItemDefinition(item.first);
            if (pItem)
            {
                str += std::format("{} {}", item.second, pItem->GetName());
            }
            else
            {
                str += std::format("{} Unknown Items", item.second);
            }
        }
    }

    return str;
}

void cpp_conv::Conveyor::AssessPosition(const cpp_conv::WorldMap& map)
{
    const Entity* pEntity = map.GetEntity(cpp_conv::grid::GetForwardPosition(m_position, m_direction));

    m_bIsCorner = cpp_conv::targeting_util::IsCornerConveyor(map, *this);
    m_bIsClockwise = cpp_conv::targeting_util::IsClockwiseCorner(map, *this);
    m_bIsCapped = !pEntity || !pEntity->SupportsInsertion(); 

    std::tie(m_iInnerMostChannel, m_eCornerDirection) = GetInnerMostCornerChannel(map, *this);

    for (int iLane = 0; iLane < (int)m_pChannels.size(); iLane++)
    {
        m_pChannels[iLane].m_LaneLength = 2;
        if (m_bIsCorner)
        {
            m_pChannels[iLane].m_LaneLength += GetInnerMostChannel() == iLane ? -1 : 1;
        }

        for (int iSlot = 0; iSlot < (int)m_pChannels[iLane].m_pSlots.size(); iSlot++)
        {
            m_pChannels[iLane].m_pSlots[iSlot].m_VisualPosition = cpp_conv::targeting_util::GetRenderPosition(map, *this, { iLane, iSlot });
        }
    }

    if (!m_bIsCorner)
    {
        if (!IsCapped())
        {
            m_pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Conveyor_Straight);
        }
        else
        {
            m_pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Conveyor_Straight_End);
        }
    }
    else
    {
        if (IsClockwiseCorner())
        {
            m_pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Conveyor_CornerClockwise);
        }
        else
        {
            m_pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Conveyor_CornerAntiClockwise);
        }
    }
}

void cpp_conv::Conveyor::AddItemToSlot(
    const cpp_conv::WorldMap& map,
    cpp_conv::Conveyor::Channel* pTargetChannel,
    int forwardTargetItemSlot,
    const ItemId pItem,
    const Entity& pSourceEntity,
    int iSourceChannel,
    int iSourceLane)
{
    ItemInstance& forwardTargetItem = pTargetChannel->m_pPendingItems[forwardTargetItemSlot];
    if (pSourceEntity.m_eEntityKind != EntityKind::Conveyor)
    {
        forwardTargetItem = { pItem, 0, 0, 0, m_uiMoveTick, false };
        return;
    }

    Vector2F position = reinterpret_cast<const Conveyor*>(&pSourceEntity)->m_pChannels[iSourceChannel].m_pSlots[iSourceLane].m_VisualPosition;
    forwardTargetItem = { pItem, position.GetX(), position.GetY(), 0, m_uiMoveTick, true };
}
