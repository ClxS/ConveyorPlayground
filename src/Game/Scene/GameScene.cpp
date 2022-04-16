#include "GameScene.h"

#include "CameraComponent.h"
#include "CameraControllerSystem.h"
#include "ConveyorComponent.h"
#include "ConveyorRenderingSystem.h"
#include "ConveyorStateDeterminationSystem.h"
#include "DescriptionComponent.h"
#include "DirectionComponent.h"
#include "Factory.h"
#include "FactoryComponent.h"
#include "FactoryRegistry.h"
#include "FactorySystem.h"
#include "NameComponent.h"
#include "PositionComponent.h"
#include "RecipeDefinition.h"
#include "RecipeRegistry.h"
#include "SequenceFormationSystem.h"
#include "SequenceProcessingSystem.h"
#include "StandaloneConveyorSystem.h"
#include "WorldEntityInformationComponent.h"
#include "MathHelpers.h"
#include "ModelComponent.h"
#include "ModelRenderSystem.h"

namespace
{
    cpp_conv::components::FactoryComponent& loadFactory(atlas::scene::EcsManager& ecs,
                                                        const atlas::scene::EntityId& ecsEntity,
                                                        const cpp_conv::Entity* entity)
    {
        const auto factoryEntity = static_cast<const cpp_conv::Factory*>(entity);
        const auto definition = cpp_conv::resources::getFactoryDefinition(factoryEntity->GetDefinitionId());
        if (definition && definition->GetModel())
        {
            auto& sprite = ecs.AddComponent<cpp_conv::components::ModelComponent>(ecsEntity);
            sprite.m_Model = definition->GetModel();
        }

        ecs.AddComponent<cpp_conv::components::NameComponent>(ecsEntity, definition->GetName().c_str());
        auto& factory = ecs.AddComponent<cpp_conv::components::FactoryComponent>(ecsEntity);
        auto size = definition->GetSize();

        factory.m_Size = {size.x(), size.y(), size.z()};
        if (definition->HasOwnOutputPipe())
        {
            auto outputPipe = definition->GetOutputPipe();
            factory.m_OutputPipe = {outputPipe.x(), outputPipe.y(), outputPipe.z()};
        }

        const auto recipeId = definition->GetProducedRecipe();
        const auto recipe = cpp_conv::resources::getRecipeDefinition(recipeId);
        if (recipe)
        {
            cpp_conv::components::FactoryComponent::Recipe componentRecipe;
            componentRecipe.m_Effort = recipe->GetEffort();
            for (auto& input : recipe->GetInputItems())
            {
                componentRecipe.m_InputItems.emplace_back(input.m_idItem, input.m_uiCount);
            }

            for (auto& output : recipe->GetOutputItems())
            {
                componentRecipe.m_OutputItems.emplace_back(output.m_idItem, output.m_uiCount);
            }

            factory.m_Recipe = componentRecipe;
        }

        return factory;
    }

    void addCameras(atlas::scene::EcsManager& ecs)
    {
        const auto cameraEntity = ecs.AddEntity();
        auto& camera = ecs.AddComponent<cpp_conv::components::LookAtCamera>(cameraEntity);
        camera.m_bIsActive = true;
        camera.m_Distance = 5.0f;
        camera.m_LookAtPoint = {5.39f, 0.0f, 0.179f};
        camera.m_Pitch = 30.0_degrees;
        camera.m_Yaw = -135.0_degrees;
    }

}

void cpp_conv::GameScene::OnEntered(atlas::scene::SceneManager& sceneManager)
{
    atlas::scene::EcsManager& ecs = GetEcsManager();

    for (const auto& entity : m_InitialisationData.m_Map->GetConveyors())
    {
        const auto ecsEntity = ecs.AddEntity();
        const auto& position = ecs.AddComponent<components::PositionComponent>(
            ecsEntity, Eigen::Vector3i(entity->m_position.x(), entity->m_position.y(),
                                       entity->m_position.z()));
        ecs.AddComponent<components::NameComponent>(ecsEntity, "Basic Conveyor");
        ecs.AddComponent<components::DescriptionComponent>(ecsEntity, "The wheels of invention");
        ecs.AddComponent<components::DirectionComponent>(ecsEntity, entity->m_Direction);
        ecs.AddComponent<components::ConveyorComponent>(ecsEntity);
        ecs.AddComponent<components::WorldEntityInformationComponent>(ecsEntity, entity->m_eEntityKind);
        m_SceneData.m_LookupGrid.PlaceEntity(position.m_Position, {1, 1, 1}, ecsEntity);
    }

    for (const auto& entity : m_InitialisationData.m_Map->GetOtherEntities())
    {
        const auto ecsEntity = ecs.AddEntity();
        ecs.AddComponent<components::WorldEntityInformationComponent>(ecsEntity, entity->m_eEntityKind);
        const auto& position = ecs.AddComponent<components::PositionComponent>(
            ecsEntity,
            Eigen::Vector3i(entity->m_position.x(), entity->m_position.y(), entity->m_position.z()));
        ecs.AddComponent<components::DirectionComponent>(ecsEntity, entity->m_Direction);

        switch (entity->m_eEntityKind)
        {
        case EntityKind::Producer:
            {
                const auto& factory = loadFactory(ecs, ecsEntity, entity);
                if (!m_SceneData.m_LookupGrid.PlaceEntity(position.m_Position, factory.m_Size, ecsEntity))
                {
                    ecs.RemoveEntity(ecsEntity);
                }
            }
            break;
        default:
            if (!m_SceneData.m_LookupGrid.PlaceEntity(position.m_Position, {1, 1, 1}, ecsEntity))
            {
                ecs.RemoveEntity(ecsEntity);
            }
            break;
        }
    }

    addCameras(ecs);

    m_InitialisationData.m_Map.reset();

    EcsScene::OnEntered(sceneManager);
}

void cpp_conv::GameScene::ConstructSystems(atlas::scene::SystemsBuilder& builder)
{
    auto conveyorProcessingGroup = builder.RegisterGroup(
        "Conveyor Processing",
        [this](atlas::scene::SystemsBuilder& groupBuilder)
        {
            groupBuilder.RegisterSystem<ConveyorStateDeterminationSystem>(m_SceneData.m_LookupGrid);
            groupBuilder.RegisterSystem<SequenceFormationSystem, ConveyorStateDeterminationSystem>(
                m_SceneData.m_LookupGrid);
            groupBuilder.RegisterSystem<SequenceProcessingSystem_Process, SequenceFormationSystem>(
                m_SceneData.m_LookupGrid);
            groupBuilder.RegisterSystem<StandaloneConveyorSystem_Process, SequenceFormationSystem>(
                m_SceneData.m_LookupGrid);
        });

    auto conveyorRealizeGroup = builder.RegisterGroup(
        "Conveyor Realization",
        {conveyorProcessingGroup},
        [this](atlas::scene::SystemsBuilder& groupBuilder)
        {
            groupBuilder.RegisterSystem<SequenceProcessingSystem_Realize>();
            groupBuilder.RegisterSystem<StandaloneConveyorSystem_Realize>();
        });

    auto cameraSystems = builder.RegisterGroup("Camera",
        [](atlas::scene::SystemsBuilder& groupBuilder)
        {
            groupBuilder.RegisterSystem<CameraControllerSystem>();
        });

    auto sceneSystems = builder.RegisterGroup(
        "General Scene Systems",
        {conveyorRealizeGroup},
        [this](atlas::scene::SystemsBuilder& groupBuilder)
        {
            groupBuilder.RegisterSystem<FactorySystem>(m_SceneData.m_LookupGrid);
        });

    auto generalRendering = builder.RegisterGroup("GeneralRendering", {sceneSystems, cameraSystems}, [](atlas::scene::SystemsBuilder& groupBuilder)
    {
        groupBuilder.RegisterSystem<ModelRenderSystem>();
        groupBuilder.RegisterSystem<ConveyorRenderingSystem>();
    });
}
