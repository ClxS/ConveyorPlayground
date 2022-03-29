#include "StandaloneConveyorSystem.h"

#include "ConveyorComponent.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

void cpp_conv::StandaloneConveyorSystem_Process::Update(atlas::scene::EcsManager& ecs)
{
    for (auto& entity : ecs.GetEntitiesWithComponents<components::ConveyorComponent>())
    {
        auto& conveyor = ecs.GetComponent<components::ConveyorComponent>(entity);
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
}

void cpp_conv::StandaloneConveyorSystem_Realize::Update(atlas::scene::EcsManager& ecs)
{
    for (const auto& entity : ecs.GetEntitiesWithComponents<components::ConveyorComponent>())
    {
        auto& conveyor = ecs.GetComponent<components::ConveyorComponent>(entity);
        for (int iChannelIdx = 0; iChannelIdx < components::c_conveyorChannels; iChannelIdx++)
        {
            auto& rChannel = conveyor.m_Channels[iChannelIdx];
            int iChannelLength = components::c_conveyorChannelSlots;
            if (conveyor.m_bIsCorner)
            {
                iChannelLength += conveyor.m_InnerMostChannel == iChannelIdx ? -1 : 1;
            }

            for (int iChannelSlot = 0; iChannelSlot <= iChannelLength - 1; iChannelSlot++)
            {
                if (!rChannel.m_pPendingItems[iChannelSlot].m_Item.IsEmpty())
                {
                    rChannel.m_pSlots[iChannelSlot].m_Item = rChannel.m_pPendingItems[iChannelSlot];
                    rChannel.m_pPendingItems[iChannelSlot].m_Item = {};
                    rChannel.m_pPendingItems[iChannelSlot].m_PreviousPosition = {};
                }
            }
        }
    }
}
