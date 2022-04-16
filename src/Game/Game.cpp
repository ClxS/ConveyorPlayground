#include "Game.h"

#include "CameraComponent.h"
#include "ConveyorComponent.h"
#include "ConveyorDefinition.h"
#include "ConveyorRegistry.h"
#include "DescriptionComponent.h"
#include "DirectionComponent.h"
#include "FactoryComponent.h"
#include "FactoryRegistry.h"
#include "GameMapLoadInterstitialScene.h"
#include "InserterRegistry.h"
#include "ItemRegistry.h"
#include "MapLoadHandler.h"
#include "ModelComponent.h"
#include "NameComponent.h"
#include "PositionComponent.h"
#include "RecipeDefinition.h"
#include "RecipeRegistry.h"
#include "SDLTileLoadHandler.h"
#include "SequenceComponent.h"
#include "WorldEntityInformationComponent.h"
#include "AtlasGame/GameHost.h"
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
    using namespace cpp_conv::components;
    ComponentRegistry::RegisterComponent<LookAtCamera>();
    ComponentRegistry::RegisterComponent<FreeCamera>();
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
}

void registerAssetBundles()
{
    using namespace atlas::resource;
    ResourceLoader::RegisterBundle<registry::CoreBundle>();
}

void registerTypeHandlers()
{
    using namespace atlas::resource;
    ResourceLoader::RegisterTypeHandler<Map>(mapAssetHandler);
    ResourceLoader::RegisterTypeHandler<cpp_conv::ConveyorDefinition>(conveyorAssetHandler);
    ResourceLoader::RegisterTypeHandler<cpp_conv::FactoryDefinition>(factoryAssetHandler);
    ResourceLoader::RegisterTypeHandler<cpp_conv::InserterDefinition>(inserterAssetHandler);
    ResourceLoader::RegisterTypeHandler<cpp_conv::ItemDefinition>(itemAssetHandler);
    ResourceLoader::RegisterTypeHandler<cpp_conv::RecipeDefinition>(recipeAssetHandler);
    ResourceLoader::RegisterTypeHandler<TileAsset>(cpp_conv::textTileLoadHandler);

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
    atlas::game::GameHost<CppConveyor> game{{"Transportadoras"}};
    return game.Run();
}
