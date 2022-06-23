#include "SequenceProcessingSystem.h"

#include "ConveyorComponent.h"
#include "ConveyorHelper.h"
#include "DirectionComponent.h"
#include "EntityLookupGrid.h"
#include "ItemPassingUtility.h"
#include "PositionHelper.h"
#include "SequenceComponent.h"
#include "AtlasGame/Scene/Components/PositionComponent.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

#if defined(DEBUG)
#define USE_VALIDATION_CHECKS
#endif

const cpp_conv::components::SequenceComponent::RealizedState& getFreshRealizedStateForTick(
    cpp_conv::components::SequenceComponent& component, const uint8_t uiLane)
{
    cpp_conv::components::SequenceComponent::RealizedState& realizedState = component.m_RealizedStates[uiLane];
    realizedState.m_RealizedMovements = 0;
    realizedState.m_HasOverridePosition = 0;
    return realizedState;
}

bool moveItemToForwardsNode(
    atlas::scene::EcsManager& ecs,
    const cpp_conv::EntityLookupGrid& grid,
    const atlas::scene::EntityId currentEntity,
    const cpp_conv::components::SequenceComponent& component,
    const int lane)
{
    const auto item = component.m_RealizedStates[lane].m_Items.Peek();
    if (item.m_Item.IsEmpty())
    {
        return false;
    }

    const Eigen::Vector2f startPosition = cpp_conv::conveyor_helper::getSlotPosition(component, component.m_Length - 1, lane, 1);

    if (!ecs.DoesEntityHaveComponents<atlas::game::scene::components::PositionComponent,
                                      cpp_conv::components::DirectionComponent>(component.m_HeadConveyor))
    {
        return false;
    }

    const auto& [position, direction] = ecs.GetComponents<
        atlas::game::scene::components::PositionComponent, cpp_conv::components::DirectionComponent>(component.m_HeadConveyor);

    const auto forwardEntity = grid.GetEntity(
        cpp_conv::position_helper::getForwardPosition(position.m_Position, direction.m_Direction));
    if (forwardEntity.IsInvalid())
    {
        return false;
    }

    return cpp_conv::item_passing_utility::tryInsertItem(ecs, grid, component.m_HeadConveyor, forwardEntity, item.m_Item, lane,
                                                         startPosition);
}

cpp_conv::SequenceProcessingSystem_Process::SequenceProcessingSystem_Process(EntityLookupGrid& lookupGrid): m_LookupGrid{lookupGrid}
{
}

void cpp_conv::SequenceProcessingSystem_Process::Update(atlas::scene::EcsManager& ecs)
{
    using components::SequenceComponent;

    for (const auto entity : ecs.GetEntitiesWithComponents<SequenceComponent>())
    {
        auto& sequence = ecs.GetComponent<SequenceComponent>(entity);

        sequence.m_CurrentTick++;
        if (sequence.m_CurrentTick < sequence.m_MoveTick)
        {
            continue;
        }

        sequence.m_CurrentTick = 0;
        for (uint8_t uiLane = 0; uiLane < components::c_conveyorChannels; uiLane++)
        {
            const SequenceComponent::RealizedState& realizedState = getFreshRealizedStateForTick(sequence, uiLane);
            SequenceComponent::PendingState& pendingState = sequence.m_PendingStates[uiLane];
            pendingState.m_PendingClears = 0;

            bool bIsLeadItemFull = (realizedState.m_Lanes & 0b1) == 1;
            if (bIsLeadItemFull)
            {
                if (moveItemToForwardsNode(ecs, m_LookupGrid, entity, sequence, uiLane))
                {
                    pendingState.m_PendingRemovals |= 0b1;
                    bIsLeadItemFull = false;
                }
            }

            const uint64_t uiNewPositions = realizedState.m_Lanes >> 1;
            uint64_t uiOverlaps = uiNewPositions & pendingState.m_PendingMoves;
            if (uiOverlaps == 0)
            {
                if (bIsLeadItemFull)
                {
                    const uint64_t uiMaxMask = (1ULL << std::countr_one(realizedState.m_Lanes)) - 1ULL;
                    pendingState.m_PendingClears = ~uiMaxMask;

                    pendingState.m_PendingMoves |= realizedState.m_Lanes >> 1;
                    pendingState.m_PendingMoves &= ~uiMaxMask;
                }
                else
                {
                    // No mid-insert collision fast path
                    pendingState.m_PendingClears = realizedState.m_Lanes;
                    pendingState.m_PendingMoves |= uiNewPositions;
                }
            }
            else
            {
                uint64_t uiMoveCandidates = realizedState.m_Lanes;
                if (bIsLeadItemFull)
                {
                    const uint64_t uiMaxMask = (1ULL << std::countr_one(realizedState.m_Lanes)) - 1ULL;
                    pendingState.m_PendingClears &= ~uiMaxMask;
                    uiMoveCandidates &= ~uiMaxMask;
                }

                do
                {
                    // Determine save area
                    uint64_t uiCollisionBit;
                    uint64_t safeRegionMask;
                    {
                        uiCollisionBit = std::countr_zero(uiOverlaps);
                        uiOverlaps &= ~(1ULL << uiCollisionBit);

                        // Everything below our collision bit is safe to move
                        safeRegionMask = (1ULL << uiCollisionBit) - 1;
                    }

                    // Perform safe move area
                    // E.g, if we had a safe region of 0b0111
                    // And move candidates 0b1010
                    // Pending moves  would be OR 0b0101
                    // Pending clears would be OR 0b0010
                    // We do not clear the 4th slot as that is our collision bit - we know a new entry is in that position
                    {
                        pendingState.m_PendingMoves |= (uiMoveCandidates >> 1) & safeRegionMask;
                        pendingState.m_PendingClears |= (uiMoveCandidates) & (((safeRegionMask << 1) | 0b1));
                        pendingState.m_PendingClears &= ~(1ULL << uiCollisionBit);

                        uiMoveCandidates &= ~((safeRegionMask << 1) | 0b1);
                        uiMoveCandidates &= ~(1ULL << uiCollisionBit);
                    }

                    //
                    {
                        // We can't move anything else until the following 0 bit
                        const uint64_t uiConsecutiveCollisions = std::countr_one(
                            uiMoveCandidates >> (uiCollisionBit + 1));
                        const uint64_t uiClearRange = (1ULL << uiConsecutiveCollisions) - 1;

                        uiMoveCandidates = (uiMoveCandidates >> (uiCollisionBit + 1) & ~uiClearRange) << (uiCollisionBit
                            + 1);
                    }
                }
                while (uiOverlaps != 0);

                if (uiMoveCandidates != 0)
                {
                    pendingState.m_PendingClears |= uiMoveCandidates;
                    pendingState.m_PendingMoves |= (uiMoveCandidates >> 1);
                }
            }
        }
    }
}

void cpp_conv::SequenceProcessingSystem_Realize::Update(atlas::scene::EcsManager& ecs)
{
    using components::SequenceComponent;

    for (const auto entity : ecs.GetEntitiesWithComponents<SequenceComponent>())
    {
        auto& sequence = ecs.GetComponent<SequenceComponent>(entity);

        for (uint8_t uiLane = 0; uiLane < c_conveyorChannels; uiLane++)
        {
            SequenceComponent::RealizedState& realizedState = sequence.m_RealizedStates[uiLane];
            SequenceComponent::PendingState& pendingState = sequence.m_PendingStates[uiLane];

            if (pendingState.m_PendingRemovals != 0)
            {
                uint64_t mutableRealizedLane = realizedState.m_Lanes;
                while (pendingState.m_PendingRemovals != 0)
                {
                    const uint64_t uiClearIndex = 1ULL << std::countr_zero(pendingState.m_PendingRemovals);
                    const uint64_t uiEarlierItemsMask = uiClearIndex - 1;
                    pendingState.m_PendingRemovals &= ~static_cast<uint64_t>(uiClearIndex);

                    const uint8_t removalIndex = std::popcount(mutableRealizedLane & uiEarlierItemsMask);
                    realizedState.m_Items.Remove(removalIndex);

                    mutableRealizedLane &= ~static_cast<uint64_t>(uiClearIndex);

                    if ((pendingState.m_PendingClears & uiClearIndex) != 0)
                    {
                        // This was being moved previously, need to clear the move as well, which will be 1 slot to the right
                        pendingState.m_PendingMoves &= ~(uiClearIndex >> 1ULL);
                    }
                }
            }

            realizedState.m_Lanes &= ~pendingState.m_PendingClears;
            realizedState.m_Lanes |= pendingState.m_PendingMoves;
            realizedState.m_RealizedMovements |= pendingState.m_PendingMoves;

            pendingState.m_PendingClears = 0;
            pendingState.m_PendingMoves = 0;

            uint64_t uiInsertions = pendingState.m_PendingInsertions;
            pendingState.m_PendingInsertions = 0;

            while (uiInsertions != 0)
            {
                const SequenceComponent::SlotItem item = pendingState.m_NewItems.Pop();
                const uint64_t uiCurrentInsertIndex = 1ULL << std::countr_zero(uiInsertions);
                if (item.m_Position.has_value())
                {
                    realizedState.m_HasOverridePosition |= uiCurrentInsertIndex;
                }

                const uint64_t uiEarlierItemsMask = uiCurrentInsertIndex - 1;
                uiInsertions &= ~static_cast<uint64_t>(uiCurrentInsertIndex);
                const uint64_t previousItemCount = std::popcount(realizedState.m_Lanes & uiEarlierItemsMask);
                realizedState.m_Items.Insert(previousItemCount, item);
            }

#ifdef USE_VALIDATION_CHECKS
            assert(static_cast<uint32_t>(std::popcount(realizedState.m_Lanes)) == realizedState.m_Items.GetSize());
#endif
        }
    }
}
