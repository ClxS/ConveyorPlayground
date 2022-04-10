#include "Game.h"
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
#include "NameComponent.h"
#include "PositionComponent.h"
#include "RecipeDefinition.h"
#include "RecipeRegistry.h"
#include "SDLTileLoadHandler.h"
#include "SequenceComponent.h"
#include "SpriteLayerComponent.h"
#include "WorldEntityInformationComponent.h"
#include "AtlasGame/GameHost.h"

#undef max
#undef min

using namespace cpp_conv::resources;

void registerComponents()
{
    using namespace atlas::resource;
    using namespace atlas::scene;
    using namespace cpp_conv::components;
    ComponentRegistry::RegisterComponent<NameComponent>();
    ComponentRegistry::RegisterComponent<DescriptionComponent>();
    ComponentRegistry::RegisterComponent<ConveyorComponent>();
    ComponentRegistry::RegisterComponent<IndividuallyProcessableConveyorComponent>();
    ComponentRegistry::RegisterComponent<DirectionComponent>();
    ComponentRegistry::RegisterComponent<FactoryComponent>();
    ComponentRegistry::RegisterComponent<PositionComponent>();
    ComponentRegistry::RegisterComponent<SequenceComponent>();
    ComponentRegistry::RegisterComponent<SpriteLayerComponent<1>>();
    ComponentRegistry::RegisterComponent<SpriteLayerComponent<2>>();
    ComponentRegistry::RegisterComponent<SpriteLayerComponent<3>>();
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
}

void loadDataAssets()
{
    loadConveyors();
    loadFactories();
    loadInserters();
    loadItems();
    loadRecipes();
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

        m_SceneManager.TransitionTo<cpp_conv::GameMapLoadInterstitialScene>(
            ResourceLoader::CreateBundleRegistryId<registry::CoreBundle>(registry::core_bundle::maps::c_bigmap));
    }
};

int gameMain(int argc, char* argv[])
{
    atlas::game::GameHost<CppConveyor> game{};
    return game.Run();
}
