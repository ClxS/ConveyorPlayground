#include "StandaloneConveyorSystem.h"

#include "ConveyorComponent.h"
#include "ConveyorHelper.h"
#include "DirectionComponent.h"
#include "EntityGrid.h"
#include "EntityLookupGrid.h"
#include "ItemPassingUtility.h"
#include "PositionComponent.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

cpp_conv::StandaloneConveyorSystem_Process::StandaloneConveyorSystem_Process(EntityLookupGrid& lookupGrid): m_LookupGrid{lookupGrid}
{
}

void cpp_conv::StandaloneConveyorSystem_Process::Update(atlas::scene::EcsManager& ecs)
{
    for (const auto& entity : ecs.GetEntitiesWithComponents<
        components::PositionComponent,
        components::DirectionComponent,
        components::ConveyorComponent,
        components::IndividuallyProcessableConveyorComponent>())
    {
        auto& conveyor = ecs.GetComponent<components::ConveyorComponent>(entity);

        // We shouldn't be able to have a sequenced conveyor which has the IndividuallyProcessableConveyorComponent
        // attached
        assert(conveyor.m_Sequence.IsInvalid());

        conveyor.m_CurrentTick++;
        if (conveyor.m_CurrentTick < conveyor.m_MoveTick)
        {
            return;
        }

        const auto& [position, direction] = ecs.GetComponents<components::PositionComponent, components::DirectionComponent>(entity);

        conveyor.m_CurrentTick = 0;
        for (int iChannelIdx = 0; iChannelIdx < components::c_conveyorChannels; iChannelIdx++)
        {
            components::ConveyorComponent::Channel& rChannel = conveyor.m_Channels[iChannelIdx];

            int iChannelLength = components::c_conveyorChannelSlots;
            if (conveyor.m_bIsCorner)
            {
                iChannelLength += conveyor.m_InnerMostChannel == iChannelIdx ? -1 : 1;
            }

            components::ConveyorComponent::PlacedItem& rLeadingItem = rChannel.m_pSlots[iChannelLength - 1].m_Item;
            if (!rLeadingItem.m_Item.IsEmpty())
            {
                const auto pForwardEntity = m_LookupGrid.GetEntity(grid::getForwardPosition(position.m_Position, direction.m_Direction));
                if (item_passing_utility::tryInsertItem(
                    ecs,
                    m_LookupGrid,
                    entity,
                    pForwardEntity,
                    rLeadingItem.m_Item,
                    iChannelIdx,
                    rChannel.m_pSlots[iChannelLength - 1].m_VisualPosition))
                {
                    rLeadingItem = {};
                }
                else
                {
                    rLeadingItem.m_bShouldAnimate = false;
                }
            }

            for (int iChannelSlot = rChannel.m_LaneLength - 2; iChannelSlot >= 0; iChannelSlot--)
            {
                components::ConveyorComponent::PlacedItem& currentItem = rChannel.m_pSlots[iChannelSlot].m_Item;
                components::ConveyorComponent::PlacedItem& forwardTargetItem = rChannel.m_pSlots[iChannelSlot + 1].m_Item;
                components::ConveyorComponent::PlacedItem& forwardPendingItem = rChannel.m_pSlots[iChannelSlot + 1].m_Item;
                if (!currentItem.m_Item.IsEmpty())
                {
                    if (forwardTargetItem.m_Item.IsEmpty() && forwardPendingItem.m_Item.IsEmpty())
                    {
                        cpp_conv::conveyor_helper::placeItemInSlot(
                            ecs,
                            conveyor,
                            rChannel.m_ChannelLane,
                            iChannelSlot + 1,
                     {
                                currentItem.m_Item,
                                rChannel.m_pSlots[iChannelSlot].m_VisualPosition
                            },
                            true);
                        currentItem = {};
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
