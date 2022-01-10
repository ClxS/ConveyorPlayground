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
    bool allowBackFill = false)
{
    PROFILE_FUNC();

    if (!pTile)
    {
        return;
    }

    // TODO[CJones] Normalize this
    if (bIsCorner)
    {
        if (bIsInnerMostChannel)
        {
            switch (direction)
            {
            case Direction::Left:
                x += cpp_conv::c_conveyorChannelSlots - iChannelSlot - 1;
                y += cpp_conv::c_conveyorChannels - iChannelIdx;
                break;
            case Direction::Up:
                x += cpp_conv::c_conveyorChannels - iChannelIdx;
                y += 1 + iChannelSlot + 1;
                break;
            case Direction::Right:
                x += 1 + iChannelSlot + 1;
                y += 1 + iChannelIdx;
                break;
            case Direction::Down:
                x += 1 + iChannelIdx;
                y += cpp_conv::c_conveyorChannelSlots - iChannelSlot - 1;
                break;
            }
        }
        else
        {
            switch (direction)
            {
            case Direction::Left:
                x += cpp_conv::c_conveyorChannelSlots - iChannelSlot + (iChannelSlot > 0 ? 1 : 0);;
                y += cpp_conv::c_conveyorChannels - iChannelIdx + (iChannelSlot == 0 ? (cornerSourceDirection == Direction::Up ? -1 : 1) : 0);
                break;
            case Direction::Right:
                x += 1 + iChannelSlot + (iChannelSlot > 0 ? -1 : 0);
                y += 1 + iChannelIdx + (iChannelSlot == 0 ? (cornerSourceDirection == Direction::Up ? -1 : 1) : 0);
                break;
            case Direction::Up:
                x += cpp_conv::c_conveyorChannels - iChannelIdx + (iChannelSlot == 0 ? (cornerSourceDirection == Direction::Left ? 1 : -1) : 0);
                y += 1 + iChannelSlot + (iChannelSlot > 0 ? -1 : 0);
                break;
            case Direction::Down:
                x += 1 + iChannelIdx + (iChannelSlot == 0 ? (cornerSourceDirection == Direction::Left ? 1 : -1) : 0);
                y += cpp_conv::c_conveyorChannelSlots - iChannelSlot + (iChannelSlot > 0 ? 1 : 0);;
                break;
            }
        }
    }
    else
    {
        switch (direction)
        {
        case Direction::Left:
            x += cpp_conv::c_conveyorChannelSlots - iChannelSlot;
            y += cpp_conv::c_conveyorChannels - iChannelIdx;
            break;
        case Direction::Up:
            x += cpp_conv::c_conveyorChannels - iChannelIdx;
            y += 1 + iChannelSlot;
            break;
        case Direction::Right:
            x += 1 + iChannelSlot;
            y += 1 + iChannelIdx;
            break;
        case Direction::Down:
            x += 1 + iChannelIdx;
            y += cpp_conv::c_conveyorChannelSlots - iChannelSlot;
            break;
        }
    }

    cpp_conv::renderer::renderAsset(kContext, pTile.get(), { x, y }, { colour.m_value | 0xFF000000 });
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
        pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Conveyor_Straight);        
    }
    else
    {
        if (cpp_conv::targeting_util::IsClockwiseCorner(kContext.m_grid, rConveyor))
        {
            pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Conveyor_CornerClockwise);
        }
        else
        {
            pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Conveyor_CornerAntiClockwise);
        }        
    }   

    transform = { x, y, cpp_conv::rotationFromDirection(direction) };
    if (pTile)
    {
        cpp_conv::renderer::renderAsset(kContext, pTile.get(), transform, colour);
    }
}

cpp_conv::Conveyor::Channel::Channel()
{
    std::fill(std::begin(m_pItems), std::end(m_pItems), cpp_conv::ItemIds::None);
    std::fill(std::begin(m_pPendingItems), std::end(m_pPendingItems), cpp_conv::ItemIds::None);
}

cpp_conv::Conveyor::Conveyor(int32_t x, int32_t y, Direction direction, ItemId pItem)
    : Entity(x, y, EntityKind::Conveyor)
    , m_direction(direction)
    , m_pSequenceId(0)
{
}

void cpp_conv::Conveyor::Tick(const SceneContext& kContext)
{
    bool bIsCornerConveyor = cpp_conv::targeting_util::IsCornerConveyor(kContext.m_grid, *this);
    int iInnerMostChannel;
    Direction eCornerDirection;
    std::tie(iInnerMostChannel, eCornerDirection) = GetInnerMostCornerChannel(kContext.m_grid, *this);

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
                rChannel.m_pPendingItems[iChannelSlot] = cpp_conv::ItemIds::None;
            }
        }
    }
}

void cpp_conv::Conveyor::Draw(RenderContext& kContext) const
{
    PROFILE_FUNC();
    Colour colour = GetColourFromId(m_pSequenceId);

    int conveyorX = m_position.m_x * 3;
    int conveyorY = m_position.m_y * 3;

    bool bIsCorner = cpp_conv::targeting_util::IsCornerConveyor(kContext.m_grid, *this);

    int iInnerMostChannel;
    Direction eCornerDirection;
    std::tie(iInnerMostChannel, eCornerDirection) = GetInnerMostCornerChannel(kContext.m_grid, *this);

    DrawConveyor(
        kContext,
        *this,
        conveyorX,
        conveyorY,
        bIsCorner,
        m_direction,
        eCornerDirection,
        colour);
;
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
                cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition> pItem = cpp_conv::resources::getItemDefinition(m_pChannels[iChannelIdx].m_pItems[iChannelSlot]);
                if (pItem)
                {
                    DrawConveyorItem(
                        kContext,
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
                        true);
                }
            }
        }
    }
}

bool cpp_conv::Conveyor::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const ItemId pItem, int iSourceChannel)
{
    cpp_conv::Conveyor::Channel* pTargetChannel = cpp_conv::targeting_util::GetTargetChannel(kContext.m_grid, pSourceEntity, *this, iSourceChannel);
    if (!pTargetChannel)
    {
        return false;
    }

    int forwardTargetItemSlot = cpp_conv::targeting_util::GetChannelTargetSlot(kContext.m_grid, pSourceEntity, *this, iSourceChannel);

    ItemId& forwardTargetItem = pTargetChannel->m_pItems[forwardTargetItemSlot];
    ItemId& forwardPendingItem = pTargetChannel->m_pPendingItems[forwardTargetItemSlot];

    // Following node is empty, we can just move there
    if (!forwardTargetItem.IsEmpty() || !forwardPendingItem.IsEmpty())
    {
        return false;
    }

    forwardPendingItem = pItem;
    return true;
}

bool cpp_conv::Conveyor::TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem)
{
    bool bIsCorner = cpp_conv::targeting_util::IsCornerConveyor(kContext.m_grid, *this);
    int iInnerMostChannel;
    Direction eCornerDirection;
    std::tie(iInnerMostChannel, eCornerDirection) = GetInnerMostCornerChannel(kContext.m_grid, *this);

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
                outItem = std::make_tuple(m_pChannels[iChannelIdx].m_pItems[iChannelSlot], 1);
                m_pChannels[iChannelIdx].m_pItems[iChannelSlot] = ItemIds::None;
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
            ItemId item = m_pChannels[iChannelIdx].m_pItems[iChannelSlot];
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
