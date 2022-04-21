#include "GameScene.h"

#include "CameraComponent.h"
#include "CameraControllerSystem.h"
#include "Constants.h"
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
#include "PostProcessSystem.h"
#include "AtlasAppHost/Application.h"
#include "AtlasRender/Renderer.h"

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

    builder.RegisterGroup("Camera",
        [](atlas::scene::SystemsBuilder& groupBuilder)
        {
            groupBuilder.RegisterSystem<CameraControllerSystem>();
        });

    builder.RegisterGroup(
        "General Scene Systems",
        {conveyorRealizeGroup},
        [this](atlas::scene::SystemsBuilder& groupBuilder)
        {
            groupBuilder.RegisterSystem<FactorySystem>(m_SceneData.m_LookupGrid);
        });

    ConstructFrameGraph();
}

void cpp_conv::GameScene::ConstructFrameGraph()
{
    using namespace constants;
    bgfx::ViewId order[] =
    {
        render_views::c_geometry,
        render_views::c_ui,
        render_views::c_postProcess,
    };
    bgfx::setViewOrder(0, BX_COUNTOF(order), order);


    atlas::render::addToFrameGraph("SetPrimaryRenderTargets",
        [this]()
        {
            const auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();
            m_RenderSystems.m_GBuffer.Initialise(width, height);

            bgfx::setState(BGFX_STATE_DEFAULT);
            bgfx::setViewName(render_views::c_geometry, "Mesh");
            bgfx::setViewClear(render_views::c_geometry, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x322e3dFF);
            bgfx::setViewRect(render_views::c_geometry, 0, 0, bgfx::BackbufferRatio::Equal);
            bgfx::setViewFrameBuffer(render_views::c_geometry, m_RenderSystems.m_GBuffer.GetHandle());

            bgfx::setViewName(render_views::c_ui, "UI Layer");
            bgfx::setViewClear(render_views::c_ui, 0);
            bgfx::setViewRect(render_views::c_ui, 0, 0, bgfx::BackbufferRatio::Equal);
            bgfx::setViewMode(render_views::c_ui, bgfx::ViewMode::Sequential);
            bgfx::setViewFrameBuffer(render_views::c_ui, m_RenderSystems.m_GBuffer.GetHandle());

            bgfx::setViewName(render_views::c_postProcess, "OutputView");
            bgfx::setViewClear(render_views::c_postProcess, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x322e3dFF);
            bgfx::setViewRect(render_views::c_postProcess, 0, 0, bgfx::BackbufferRatio::Equal);
            bgfx::setViewFrameBuffer(render_views::c_postProcess, BGFX_INVALID_HANDLE);
        },
        [this]()
        {
            const auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();
            m_RenderSystems.m_GBuffer.EnsureSize(width, height);
        });

    atlas::render::addToFrameGraph("RenderWorld",
        [this]()
        {
            DirectInitialiseSystem(m_RenderSystems.m_CameraRenderSystem);
            DirectInitialiseSystem(m_RenderSystems.m_ModelRenderer);
            DirectInitialiseSystem(m_RenderSystems.m_ConveyorRenderer);
        },
        [this]()
        {
            bgfx::setState(BGFX_STATE_DEFAULT);
            DirectRunSystem(m_RenderSystems.m_CameraRenderSystem);
            DirectRunSystem(m_RenderSystems.m_ModelRenderer);
            DirectRunSystem(m_RenderSystems.m_ConveyorRenderer);
        });

    atlas::render::addToFrameGraph("UI",
       [this]()
       {
           DirectInitialiseSystem(m_RenderSystems.m_UIController);
       },
       [this]()
       {
           DirectRunSystem(m_RenderSystems.m_UIController);
       });

    atlas::render::addToFrameGraph("FXAA",
        [this]()
        {
            DirectInitialiseSystem(m_RenderSystems.m_PostProcess, &m_RenderSystems.m_GBuffer);
        },
        [this]()
        {
            DirectRunSystem(m_RenderSystems.m_PostProcess);
        });
}
