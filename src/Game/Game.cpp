#include "Game.h"

#include "AssetHandlerCommon.h"
#include "Constants.h"
#include "ConveyorComponent.h"
#include "ConveyorDefinition.h"
#include "ConveyorRegistry.h"
#include "DescriptionComponent.h"
#include "DirectionComponent.h"
#include "FactoryComponent.h"
#include "FactoryRegistry.h"
#include "GameMapLoadInterstitialScene.h"
#include "InserterDefinition.h"
#include "InserterRegistry.h"
#include "ItemDefinition.h"
#include "ItemRegistry.h"
#include "MapLoadHandler.h"
#include "ModelComponent.h"
#include "NameComponent.h"
#include "RecipeDefinition.h"
#include "RecipeRegistry.h"
#include "SDLTileLoadHandler.h"
#include "SequenceComponent.h"
#include "SolarBodyComponent.h"
#include "StorageComponent.h"
#include "WorldEntityInformationComponent.h"
#include "AtlasGame/GameHost.h"
#include "AtlasGame/Scene/Components/PositionComponent.h"
#include "AtlasGame/Scene/Components/Cameras/FreeCameraComponent.h"
#include "AtlasGame/Scene/Components/Cameras/LookAtCameraComponent.h"
#include "AtlasGame/Scene/Components/Cameras/SphericalLookAtCameraComponent.h"
#include "AtlasGame/Scene/Components/Lighting/DirectionalLightComponent.h"
#include "AtlasRender/AssetTypes/ModelAsset.h"
#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasRender/AssetTypes/TextureAsset.h"

#undef max
#undef min

using namespace cpp_conv::resources;

void registerComponents()
{
    using namespace atlas::resource;
    using namespace atlas::scene;
    using namespace atlas::game::scene::components;
    using namespace cameras;
    using namespace cpp_conv::components;
    ComponentRegistry::RegisterComponent<LookAtCameraComponent>();
    ComponentRegistry::RegisterComponent<SphericalLookAtCameraComponent>();
    ComponentRegistry::RegisterComponent<SphericalLookAtCameraComponent_Private>();
    ComponentRegistry::RegisterComponent<SquareSolarBodyComponent>();
    ComponentRegistry::RegisterComponent<HexagonSolarBodyComponent>();
    ComponentRegistry::RegisterComponent<FreeCameraComponent>();
    ComponentRegistry::RegisterComponent<NameComponent>();
    ComponentRegistry::RegisterComponent<DescriptionComponent>();
    ComponentRegistry::RegisterComponent<ConveyorComponent>();
    ComponentRegistry::RegisterComponent<IndividuallyProcessableConveyorComponent>();
    ComponentRegistry::RegisterComponent<DirectionComponent>();
    ComponentRegistry::RegisterComponent<FactoryComponent>();
    ComponentRegistry::RegisterComponent<PositionComponent>();
    ComponentRegistry::RegisterComponent<SequenceComponent>();
    ComponentRegistry::RegisterComponent<ModelComponent>();
    ComponentRegistry::RegisterComponent<WorldEntityInformationComponent>();
    ComponentRegistry::RegisterComponent<StorageComponent>();
    ComponentRegistry::RegisterComponent<DirectionalLightComponent>();
}

void registerAssetBundles()
{
    using namespace atlas::resource;
    ResourceLoader::RegisterBundle<registry::CoreBundle>();
}

template<typename TDefinition>
void registerDefinitionTypeHandler()
{
    using namespace atlas::resource;
    ResourceLoader::RegisterTypeHandler<TDefinition>(asset_handler_common::deserializingAssetHandler<TDefinition>);
}

void registerTypeHandlers()
{
    using namespace atlas::resource;
    ResourceLoader::RegisterTypeHandler<Map>(mapAssetHandler);
    ResourceLoader::RegisterTypeHandler<TileAsset>(cpp_conv::textTileLoadHandler);

    registerDefinitionTypeHandler<cpp_conv::ConveyorDefinition>();
    registerDefinitionTypeHandler<cpp_conv::FactoryDefinition>();
    registerDefinitionTypeHandler<cpp_conv::InserterDefinition>();
    registerDefinitionTypeHandler<cpp_conv::ItemDefinition>();
    registerDefinitionTypeHandler<cpp_conv::RecipeDefinition>();

    ResourceLoader::RegisterTypeHandler<atlas::render::VertexShader>(atlas::render::vertexShaderLoadHandler);
    ResourceLoader::RegisterTypeHandler<atlas::render::FragmentShader>(atlas::render::fragmentShaderLoadHandler);
    ResourceLoader::RegisterTypeHandler<atlas::render::ShaderProgram>(atlas::render::shaderProgramLoadHandler);
    ResourceLoader::RegisterTypeHandler<atlas::render::TextureAsset>(atlas::render::textureLoadHandler);
    ResourceLoader::RegisterTypeHandler<atlas::render::ModelAsset>(atlas::render::modelLoadHandler);
    ResourceLoader::RegisterTypeHandler<atlas::render::MeshAsset>(atlas::render::meshLoadHandler);
}

void loadDataAssets()
{
    loadConveyors();
    loadFactories();
    loadInserters();
    loadItems();
    loadRecipes();
}

void setBgfxSettings()
{
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x322e3dFF, 1.0f, 0);
    bgfx::setViewClear(cpp_conv::constants::render_views::c_geometry, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x322e3dFF, 1.0f, 0);
}

class CppConveyor final : public atlas::game::GameImplementation
{
public:
    void OnStartup() override
    {
        using namespace atlas::resource;
        using namespace atlas::scene;

        registerComponents();
        registerTypeHandlers();
        registerAssetBundles();
        loadDataAssets();
        setBgfxSettings();

        m_SceneManager.TransitionTo<cpp_conv::GameMapLoadInterstitialScene>(
            ResourceLoader::CreateBundleRegistryId<registry::CoreBundle>(registry::core_bundle::maps::c_simple));
    }
};

int gameMain(int argc, char* argv[])
{
    atlas::game::GameHost<CppConveyor> game{{"Transportadoras", 30}};
    return game.Run();
}
