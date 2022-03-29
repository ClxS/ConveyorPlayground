#include "FactorySystem.h"

#include "ConveyorComponent.h"
#include "DirectionComponent.h"
#include "EntityGrid.h"
#include "EntityLookupGrid.h"
#include "FactoryComponent.h"
#include "ItemPassingUtility.h"
#include "PositionComponent.h"
#include "Transform2D.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

namespace
{
    bool isReadyToProduce(const cpp_conv::components::FactoryComponent& factory)
    {
        return factory.m_RemainingCurrentProductionEffort == 0;
    }

    bool trySatisfyRecipeInput(cpp_conv::components::FactoryComponent& factory)
    {
        if (!factory.m_Recipe.has_value())
        {
            return false;
        }

        if (factory.m_Recipe->m_InputItems.empty())
        {
            return true;
        }

        for (const auto& pItem : factory.m_Recipe->m_InputItems)
        {
            bool bIsMet = false;
            for (auto& rStorageItem : factory.m_InputItems.GetItems())
            {
                if (rStorageItem.m_pItem == pItem.m_Item && rStorageItem.m_pCount >= pItem.m_Count)
                {
                    bIsMet = true;
                    break;
                }
            }

            if (!bIsMet)
            {
                return false;
            }
        }

        for (const auto& pItem : factory.m_Recipe->m_InputItems)
        {
            factory.m_InputItems.TryTake(pItem.m_Item, pItem.m_Count);
        }

        return true;
    }

    bool produceItems(cpp_conv::components::FactoryComponent& factory)
    {
        if (!factory.m_Recipe.has_value())
        {
            return false;
        }

        const auto& vOutputItems = factory.m_Recipe->m_OutputItems;
        if (vOutputItems.empty())
        {
            return true;
        }

        for (const auto& pItem : vOutputItems)
        {
            if (!factory.m_OutputItems.CouldInsert(pItem.m_Item, pItem.m_Count))
            {
                return false;
            }
        }

        for (const auto& pItem : vOutputItems)
        {
            factory.m_OutputItems.TryInsert(pItem.m_Item, pItem.m_Count);
        }

        return true;
    }

    void runProductionCycle(cpp_conv::components::FactoryComponent& factory)
    {
        if (!factory.m_bIsDemandSatisfied)
        {
            if (!trySatisfyRecipeInput(factory))
            {
                return;
            }

            factory.m_bIsDemandSatisfied = true;
        }

        if (!isReadyToProduce(factory))
        {
            factory.m_RemainingCurrentProductionEffort -= factory.m_ProductionRate;
            if (!isReadyToProduce(factory))
            {
                return;
            }
        }

        if (produceItems(factory))
        {
            factory.m_bIsDemandSatisfied = false;
        }
    }

    [[nodiscard]] Eigen::Vector2i rotate(const Rotation rotation, Eigen::Vector2i input, Eigen::Vector2i size)
    {
        Eigen::Vector2i c_offset(1, 1);

        size -= c_offset;
        switch (rotation)
        {
        case Rotation::Deg90: return {size.y() - input.y(), input.x()};
        case Rotation::Deg180: return {size.x() - input.x(), size.y() - input.y()};
        case Rotation::Deg270: return {size.y() - input.y(), size.x() - input.x()};
        case Rotation::DegZero: break;
        }

        return input;
    }

    [[nodiscard]] Eigen::Vector2i getXy(Eigen::Vector3i input)
    {
        return {input.x(), input.y()};
    }

    void runOutputCycle(
        atlas::scene::EcsManager& ecs,
        const cpp_conv::EntityLookupGrid& grid,
        const atlas::scene::EntityId entity,
        cpp_conv::components::FactoryComponent& factory)
    {
        if (!factory.m_OutputPipe.has_value() ||
            factory.m_OutputItems.IsEmpty() ||
            !ecs.DoesEntityHaveComponents<cpp_conv::components::PositionComponent,
                                          cpp_conv::components::DirectionComponent>(entity))
        {
            return;
        }

        const auto& [position, direction] = ecs.GetComponents<
            cpp_conv::components::PositionComponent,
            cpp_conv::components::DirectionComponent>(entity);

        auto rotatedXy = rotate(
            cpp_conv::rotationFromDirection(direction.m_Direction),
            getXy(factory.m_OutputPipe.value()),
            getXy(factory.m_Size));

        Eigen::Vector3i pipe = {
            rotatedXy.x(),
            rotatedXy.y(),
            factory.m_OutputPipe.value().z()
        };
        pipe += position.m_Position;

        const auto targetEntity = grid.GetEntity(cpp_conv::grid::getForwardPosition(pipe, direction.m_Direction));
        if (targetEntity.IsInvalid() || !cpp_conv::item_passing_utility::entitySupportsInsertion(ecs, targetEntity))
        {
            return;
        }

        auto& vContainerItems = factory.m_OutputItems.GetItems();
        auto itItems = vContainerItems.begin();
        while (itItems != vContainerItems.end())
        {
            for (uint32_t i = 0; i < itItems->m_pCount; ++i)
            {
                if (!cpp_conv::item_passing_utility::tryInsertItem(
                    ecs,
                    grid,
                    entity,
                    targetEntity,
                    itItems->m_pItem,
                    (factory.m_Tick + i) % cpp_conv::components::c_conveyorChannels,
                    {}))

                {
                    itItems->m_pCount--;
                }
            }

            if (itItems->m_pCount == 0)
            {
                itItems = vContainerItems.erase(itItems);
            }
            else
            {
                break;
            }
        }
    }
}

void cpp_conv::FactorySystem::Update(atlas::scene::EcsManager& ecs)
{
    for (const auto entity : ecs.GetEntitiesWithComponents<components::FactoryComponent>())
    {
        auto& factory = ecs.GetComponent<components::FactoryComponent>(entity);
        factory.m_Tick++;
        runProductionCycle(factory);
        runOutputCycle(ecs, m_LookupGrid, entity, factory);
    }
}
