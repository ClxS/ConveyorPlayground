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
    int x,
    int y,
    int iChannelIdx,
    int iChannelSlot,
    bool bIsCorner,
    bool bIsInnerMostChannel,
    Direction direction,
    Direction cornerSourceDirection,
    cpp_conv::Colour colour,
    const cpp_conv::ItemInstance& item,
    bool allowBackFill = false)
{
    PROFILE_FUNC();

    if (!pTile)
    {
        return; 
    }

    float fLerpFactor = 1.0f;
    Vector2F previousPosition = {};
    if (item.m_bShouldAnimate && item.m_TargetTick != 0)
    {
        fLerpFactor = item.m_CurrentTick / (float)item.m_TargetTick;
        previousPosition = { item.m_PreviousX, item.m_PreviousY };
    }
     
    Vector2F position = cpp_conv::targeting_util::GetRenderPosition(kContext.m_rMap, rConveyor, { iChannelIdx, iChannelSlot }, item.m_bShouldAnimate, fLerpFactor, previousPosition);
    cpp_conv::renderer::renderAsset(kContext, pTile.get(), { position.GetX(), position.GetY() }, { colour.m_value | 0xFF000000 });
}
 
void DrawConveyor(
    cpp_conv::RenderContext& kContext,
    const cpp_conv::Conveyor& rConveyor,
    int x,
    int y,
    bool bIsCorner,
    Direction direction,
    Direction cornerSourceDirection,
    cpp_conv::Colour colour)
{
    PROFILE_FUNC();
    cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> pTile = nullptr;
    cpp_conv::Transform2D transform = {};

    if (!bIsCorner)
    {
        auto pEntity = kContext.m_rMap.GetEntity(cpp_conv::grid::GetForwardPosition(rConveyor.m_position, direction));

        if (pEntity && pEntity->SupportsInsertion())
        {
            pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Conveyor_Straight);
        }
        else
        {
            pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Conveyor_Straight_End);
        }
    }
    else
    {
        if (cpp_conv::targeting_util::IsClockwiseCorner(kContext.m_rMap, rConveyor))
        {
            pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Conveyor_CornerClockwise);
        }
        else
        {
            pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Conveyor_CornerAntiClockwise);
        }        
    }   

    transform = { (float)x, (float)y, cpp_conv::rotationFromDirection(direction) };
    if (pTile)
    {
        cpp_conv::renderer::renderAsset(kContext, pTile.get(), transform, colour);
    }
}

cpp_conv::Conveyor::Channel::Channel(int channelLane)
    : m_ChannelLane(channelLane)
{
    std::fill(std::begin(m_pItems), std::end(m_pItems), ItemInstance::Empty());
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
    bool bIsCornerConveyor = cpp_conv::targeting_util::IsCornerConveyor(kContext.m_rMap, *this);
    int iInnerMostChannel;
    Direction eCornerDirection;
    std::tie(iInnerMostChannel, eCornerDirection) = GetInnerMostCornerChannel(kContext.m_rMap, *this);

    for (int iChannelIdx = 0; iChannelIdx < cpp_conv::c_conveyorChannels; iChannelIdx++)
    {
        cpp_conv::Conveyor::Channel& rChannel = m_pChannels[iChannelIdx];

        int iChannelLength = cpp_conv::c_conveyorChannelSlots;
        if (bIsCornerConveyor)
        {
            iChannelLength += iInnerMostChannel == iChannelIdx ? -1 : 1;
        }

        for (int iChannelSlot = 0; iChannelSlot <= iChannelLength - 1; iChannelSlot++)
        { 
            if (!rChannel.m_pPendingItems[iChannelSlot].IsEmpty())
            {
                rChannel.m_pItems[iChannelSlot] = rChannel.m_pPendingItems[iChannelSlot];
                rChannel.m_pPendingItems[iChannelSlot] = ItemInstance::Empty();
            }

            if (!rChannel.m_pItems[iChannelSlot].IsEmpty())
            {
                ItemInstance& rItem = rChannel.m_pItems[iChannelSlot];
                if (rItem.m_CurrentTick < rChannel.m_pItems[iChannelSlot].m_TargetTick)
                {
                    ++rItem.m_CurrentTick;
                }
            }
        }
    }
}

void cpp_conv::Conveyor::Draw(RenderContext& kContext) const
{
    PROFILE_FUNC();
    Colour colour = GetColourFromId(m_pSequenceId);

    int conveyorX = m_position.GetX() * 4;
    int conveyorY = m_position.GetY() * 4;

    bool bIsCorner = cpp_conv::targeting_util::IsCornerConveyor(kContext.m_rMap, *this);

    int iInnerMostChannel;
    Direction eCornerDirection;
    std::tie(iInnerMostChannel, eCornerDirection) = GetInnerMostCornerChannel(kContext.m_rMap, *this);

    switch (kContext.m_uiCurrentDrawPass)
    {
    case 0:
    {
        DrawConveyor(
            kContext,
            *this,
            conveyorX,
            conveyorY,
            bIsCorner,
            m_direction,
            eCornerDirection,
            colour);
        break;
    }
    case 1:
    {
        for (int iChannelIdx = 0; iChannelIdx < c_conveyorChannels; ++iChannelIdx)
        {
            int iChannelLength = cpp_conv::c_conveyorChannelSlots;
            if (bIsCorner)
            {
                iChannelLength += iInnerMostChannel == iChannelIdx ? -1 : 1;
            }

            for (int iChannelSlot = 0; iChannelSlot < iChannelLength; ++iChannelSlot)
            {
                if (!m_pChannels[iChannelIdx].m_pItems[iChannelSlot].IsEmpty())
                {
                    const ItemInstance& item = m_pChannels[iChannelIdx].m_pItems[iChannelSlot];
                    cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition> pItem = cpp_conv::resources::getItemDefinition(item.m_Item);
                    if (pItem)
                    {
                        DrawConveyorItem(
                            kContext,
                            *this,
                            pItem->GetTile(),
                            conveyorX,
                            conveyorY,
                            iChannelIdx,
                            iChannelSlot,
                            bIsCorner,
                            iInnerMostChannel == iChannelIdx,
                            m_direction,
                            eCornerDirection,
                            colour,
                            item,
                            true);
                    }
                }
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

    ItemInstance& forwardTargetItem = pTargetChannel->m_pItems[forwardTargetItemSlot];
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
    bool bIsCorner = cpp_conv::targeting_util::IsCornerConveyor(kContext.m_rMap, *this);
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
            if (!m_pChannels[iChannelIdx].m_pItems[iChannelSlot].IsEmpty())
            {
                outItem = std::make_tuple(m_pChannels[iChannelIdx].m_pItems[iChannelSlot].m_Item, 1);
                m_pChannels[iChannelIdx].m_pItems[iChannelSlot] = ItemInstance::Empty();
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
            ItemId item = m_pChannels[iChannelIdx].m_pItems[iChannelSlot].m_Item;
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

    Vector2F position = cpp_conv::targeting_util::GetRenderPosition(
        map,
        *reinterpret_cast<const Conveyor*>(&pSourceEntity),
        { iSourceChannel, iSourceLane });

    forwardTargetItem = { pItem, position.GetX(), position.GetY(), 0, m_uiMoveTick, true };
}
