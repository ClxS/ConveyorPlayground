#include "GameScene.h"

#include "Constants.h"
#include "ConveyorComponent.h"
#include "ConveyorRenderingSystem.h"
#include "ConveyorStateDeterminationSystem.h"
#include "Factory.h"
#include "FactoryComponent.h"
#include "FactoryRegistry.h"
#include "FactorySystem.h"
#include "ModelComponent.h"
#include "ModelRenderSystem.h"
#include "NameComponent.h"
#include "PostProcessSystem.h"
#include "RecipeDefinition.h"
#include "RecipeRegistry.h"
#include "SequenceFormationSystem.h"
#include "SequenceProcessingSystem.h"
#include "SolarBodyComponent.h"
#include "StandaloneConveyorSystem.h"
#include "Storage.h"
#include "StorageComponent.h"
#include "AtlasAppHost/Application.h"
#include "AtlasGame/Scene/Components/Cameras/LookAtCameraComponent.h"
#include "AtlasGame/Scene/Components/Cameras/SphericalLookAtCameraComponent.h"
#include "AtlasGame/Scene/Components/Lighting/DirectionalLightComponent.h"
#include "AtlasGame/Scene/Systems/Cameras/CameraControllerSystem.h"
#include "AtlasRender/Renderer.h"
#include "AtlasRender/Debug/debugdraw.h"
#include "AtlasResource/ResourceLoader.h"
#include "SolarBodies/SolarBodyGeneration.h"

using namespace cpp_conv::components;
using namespace cpp_conv::resources::registry;
using namespace atlas::resource;
using namespace atlas::render;

namespace
{
    void loadFactory(
        cpp_conv::EntityLookupGrid& grid,
        const Eigen::Vector3i& position,
        atlas::scene::EcsManager& ecs,
        const atlas::scene::EntityId& ecsEntity,
        const cpp_conv::Entity* entity)
    {
        using namespace cpp_conv::constants::render_masks;

        const auto factoryEntity = static_cast<const cpp_conv::Factory*>(entity);
        const auto definition = cpp_conv::resources::getFactoryDefinition(factoryEntity->GetDefinitionId());
        if (definition && definition->GetModel())
        {
            ecs.AddComponent<ModelComponent>(ecsEntity, definition->GetModel(), c_generalGeometry | c_shadowCaster);
        }

        ecs.AddComponent<NameComponent>(ecsEntity, definition->GetName().c_str());
        auto& factory = ecs.AddComponent<FactoryComponent>(ecsEntity);
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
            FactoryComponent::Recipe componentRecipe;
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

        if (!grid.PlaceEntity(position, factory.m_Size, ecsEntity))
        {
            ecs.RemoveEntity(ecsEntity);
        }
    }

    void loadStorage(
        cpp_conv::EntityLookupGrid& grid,
        const Eigen::Vector3i& position,
        atlas::scene::EcsManager& ecs,
        const atlas::scene::EntityId& ecsEntity,
        const cpp_conv::Entity* entity)
    {
        using namespace cpp_conv::constants::render_masks;

        const auto storageEntity = static_cast<const cpp_conv::Storage*>(entity);
        ecs.AddComponent<NameComponent>(ecsEntity, "Storage");
        ecs.AddComponent<ModelComponent>(
            ecsEntity,
            ResourceLoader::LoadAsset<CoreBundle, ModelAsset>(core_bundle::assets::others::c_Barrel),
            c_generalGeometry | c_shadowCaster);

        auto& storage = ecs.AddComponent<StorageComponent>(ecsEntity);
        storage.m_ItemContainer.Initialise(
            storageEntity->GetContainer().GetMaxCapacity(),
            storageEntity->GetContainer().GetMaxStackSize(),
            storageEntity->GetContainer().OnlyAllowsUniqueStacks());

        if (!grid.PlaceEntity(position, { 1, 1, 1 }, ecsEntity))
        {
            ecs.RemoveEntity(ecsEntity);
        }
    }

    void loadLaunchPad(
        cpp_conv::EntityLookupGrid& grid,
        const Eigen::Vector3i& position,
        atlas::scene::EcsManager& ecs,
        const atlas::scene::EntityId& ecsEntity,
        const cpp_conv::Entity* entity)
    {
        using namespace cpp_conv::constants::render_masks;
        ecs.AddComponent<NameComponent>(ecsEntity, "Launchpad");
        ecs.AddComponent<ModelComponent>(ecsEntity,
            ResourceLoader::LoadAsset<CoreBundle, ModelAsset>(core_bundle::assets::others::c_LaunchPad),
            c_generalGeometry | c_shadowCaster | c_clipCasterGeometry);

        if (!grid.PlaceEntity(position, { 10, 4, 10 }, ecsEntity))
        {
            ecs.RemoveEntity(ecsEntity);
        }
    }

    /*
    *struct SphericalLookAtCamera : public CameraComponent
    {
        Eigen::Vector3f m_SphericalCentre;
        float m_SphericalCentreDistance;
        float m_LookAtPhi;
        float m_LookAtTheta;
        float m_Distance;
    };
     */

    void addCameras(atlas::scene::EcsManager& ecs)
    {
        auto cameraEntity = ecs.AddEntity();
        ecs.AddComponent<atlas::game::scene::components::cameras::SphericalLookAtCameraComponent_Private>(cameraEntity);
        auto& camera = ecs.AddComponent<atlas::game::scene::components::cameras::SphericalLookAtCameraComponent>(cameraEntity);
        camera.m_bIsRenderActive = true;
        camera.m_bIsControlActive = true;
        camera.m_SphericalCentre = { 0.0f, 0.0f, 0.0f };
        camera.m_SphericalCentreDistance = 10.1f;
        camera.m_LookAtPitch.SetDegrees(80.0f, 30.0f, 150.0f, atlas::maths_helpers::Angle::WrapMode::Clamp);
        camera.m_LookAtYaw.SetDegrees(0.0f, atlas::maths_helpers::Angle::WrapMode::Wrap);
        camera.m_CameraPitch.SetDegrees(
            25.0f,
            10.0f,
            75.0f,
            atlas::maths_helpers::Angle::WrapMode::Clamp);
        camera.m_Distance = 1.0f;

        cameraEntity = ecs.AddEntity();
        auto& camera2 = ecs.AddComponent<atlas::game::scene::components::cameras::LookAtCameraComponent>(cameraEntity);
        camera2.m_bIsRenderActive = false;
        camera2.m_bIsControlActive = false;
        camera2.m_Distance = 25.0f;
        camera2.m_LookAtPoint = {0.0f, 0.0f, 0.0f};
        camera2.m_Pitch = 30.0_degrees;
        camera2.m_Yaw = -135.0_degrees;
    }

    void addLights(atlas::scene::EcsManager& ecs)
    {
        const auto lightEntity = ecs.AddEntity();
        auto& light = ecs.AddComponent<atlas::game::scene::components::cameras::DirectionalLightComponent>(lightEntity);
        light.m_LightDirection = {0.08f, -0.5, -0.70f };
        light.m_LightColour = {1.0, 1.0, 1.0, 1.0f};

        light.m_LightDirection.normalize();
    }

    void addGround(atlas::scene::EcsManager& ecs)
    {
        const auto ground = ecs.AddEntity();
        ecs.AddComponent<SquareSolarBodyComponent>(ground, createSolarBody(cpp_conv::util::solar_bodies::SolarBodySpecification::CrateredMoon));
    }
}

void cpp_conv::GameScene::OnEntered(atlas::scene::SceneManager& sceneManager)
{
    atlas::scene::EcsManager& ecs = GetEcsManager();

    addGround(ecs);
    addCameras(ecs);
    addLights(ecs);

    /*for (const auto& entity : m_InitialisationData.m_Map->GetConveyors())
    {
        const auto ecsEntity = ecs.AddEntity();
        const auto& position = ecs.AddComponent<PositionComponent>(
            ecsEntity, Eigen::Vector3i(entity->m_position.x(), entity->m_position.y(),
                                       entity->m_position.z()));
        ecs.AddComponent<NameComponent>(ecsEntity, "Basic Conveyor");
        ecs.AddComponent<DescriptionComponent>(ecsEntity, "The wheels of invention");
        ecs.AddComponent<DirectionComponent>(ecsEntity, entity->m_Direction);
        ecs.AddComponent<ConveyorComponent>(ecsEntity);
        ecs.AddComponent<WorldEntityInformationComponent>(ecsEntity, entity->m_eEntityKind);
        m_SceneData.m_LookupGrid.PlaceEntity(position.m_Position, {1, 1, 1}, ecsEntity);
    }

    static std::map<EntityKind, std::function<void(EntityLookupGrid&,
            const Eigen::Vector3i&,
            atlas::scene::EcsManager&,
            const atlas::scene::EntityId&,
            const Entity*)>> handlers =
    {
        {EntityKind::Producer, &loadFactory},
        {EntityKind::Storage, &loadStorage},
        {EntityKind::LaunchPad, &loadLaunchPad},
    };

    for (const auto& entity : m_InitialisationData.m_Map->GetOtherEntities())
    {
        auto handlerIt = handlers.find(entity->m_eEntityKind);
        if (handlerIt == handlers.end())
        {
            continue;
        }

        const auto ecsEntity = ecs.AddEntity();
        ecs.AddComponent<WorldEntityInformationComponent>(ecsEntity, entity->m_eEntityKind);
        const auto& position = ecs.AddComponent<PositionComponent>(
            ecsEntity,
            Eigen::Vector3i(entity->m_position.x(), entity->m_position.y(), entity->m_position.z()));
        ecs.AddComponent<DirectionComponent>(ecsEntity, entity->m_Direction);

        (*handlerIt).second(m_SceneData.m_LookupGrid, position.m_Position, ecs, ecsEntity, entity);
    }*/

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
            groupBuilder.RegisterSystem<atlas::game::scene::systems::cameras::CameraControllerSystem>();
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
    const auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();
    m_RenderSystems.m_GBuffer.Initialise(width, height);

    bgfx::setViewName(render_views::c_geometry, "Geometry Drawn");
    bgfx::setViewClear(render_views::c_geometry, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x322e3dFF);
    setViewRect(render_views::c_geometry, 0, 0, bgfx::BackbufferRatio::Equal);
    setViewFrameBuffer(render_views::c_geometry, m_RenderSystems.m_GBuffer.GetHandle());

    bgfx::setViewName(render_views::c_ui, "UI Layer");
    bgfx::setViewClear(render_views::c_ui, 0);
    setViewRect(render_views::c_ui, 0, 0, bgfx::BackbufferRatio::Equal);
    setViewMode(render_views::c_ui, bgfx::ViewMode::Sequential);
    bgfx::setViewFrameBuffer(render_views::c_ui, BGFX_INVALID_HANDLE);

    addToFrameGraph("BufferPrep", [](){},
                    [this]()
                    {
                        const auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();
                        m_RenderSystems.m_GBuffer.EnsureSize(width, height);
                    });

    AddToFrameGraph("ShadowPass", &m_RenderSystems.m_ShadowPass);
    AddToFrameGraph("GeometryPass", &m_RenderSystems.m_GeometryPass);
    AddToFrameGraph("PostGeometryPass", &m_RenderSystems.m_PostGeometry, &m_RenderSystems.m_GBuffer);
    AddToFrameGraph("UI", &m_RenderSystems.m_UI, &m_RenderSystems.m_GeometryPass.m_CameraViewProjectionUpdateSystem);
}

void cpp_conv::GameScene::RenderSystems::ShadowPass::Initialise(atlas::scene::EcsManager& ecsManager)
{
    m_ShadowMapping.Initialise(ecsManager);
}

void cpp_conv::GameScene::RenderSystems::ShadowPass::Update(atlas::scene::EcsManager& ecsManager)
{
    atlas::scene::SystemsManager::Update(ecsManager, &m_ShadowMapping);
}

void cpp_conv::GameScene::RenderSystems::GeometryPass::Initialise(atlas::scene::EcsManager& ecsManager)
{
    m_CameraViewProjectionUpdateSystem.Initialise(ecsManager);
    m_LightingSystem.Initialise(ecsManager);
    m_ClippedSurfaceRenderSystem.Initialise(ecsManager);
    m_ModelRenderer.Initialise(ecsManager);
    m_ConveyorRenderer.Initialise(ecsManager);
}

void cpp_conv::GameScene::RenderSystems::GeometryPass::Update(atlas::scene::EcsManager& ecsManager)
{
    bgfx::setState(BGFX_STATE_DEFAULT);
    debug::debug_draw::begin(constants::render_views::c_geometry);

    {
        bgfx::setMarker("CameraViewProjectionUpdateSystem");
        atlas::scene::SystemsManager::Update(ecsManager, &m_CameraViewProjectionUpdateSystem);
    }
    {
        bgfx::setMarker("LightingSystem");
        atlas::scene::SystemsManager::Update(ecsManager, &m_LightingSystem);
    }
    {
        bgfx::setMarker("ClippedSurfaceRenderingSystem");
        atlas::scene::SystemsManager::Update(ecsManager, &m_ClippedSurfaceRenderSystem);
    }
    {
        bgfx::setMarker("ModelRenderingSystem");
        atlas::scene::SystemsManager::Update(ecsManager, &m_ModelRenderer);
    }

    {
        bgfx::setMarker("ConveyorRenderingSystem");
        atlas::scene::SystemsManager::Update(ecsManager, &m_ConveyorRenderer);
    }

    debug::debug_draw::end();
}

void cpp_conv::GameScene::RenderSystems::PostGeometry::Initialise(atlas::scene::EcsManager& ecsManager, const FrameBuffer* gbuffer)
{
    m_PostProcess.Initialise(ecsManager, gbuffer);
}

void cpp_conv::GameScene::RenderSystems::PostGeometry::Update(atlas::scene::EcsManager& ecsManager)
{
    bgfx::setState(BGFX_STATE_DEFAULT);
    atlas::scene::SystemsManager::Update(ecsManager, &m_PostProcess);
}

void cpp_conv::GameScene::RenderSystems::UI::Initialise(atlas::scene::EcsManager& ecsManager, atlas::game::scene::systems::cameras::CameraViewProjectionUpdateSystem* pCameraRenderer)
{
    m_UIController.Initialise(ecsManager);
    m_DebugUI.Initialise(ecsManager, pCameraRenderer);
}

void cpp_conv::GameScene::RenderSystems::UI::Update(atlas::scene::EcsManager& ecsManager)
{
    atlas::scene::SystemsManager::Update(ecsManager, &m_UIController);
    atlas::scene::SystemsManager::Update(ecsManager, &m_DebugUI);
}
