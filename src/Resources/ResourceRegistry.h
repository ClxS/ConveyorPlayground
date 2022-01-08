#pragma once

namespace cpp_conv::resources::registry
{
    struct RegistryId
    {
        int m_index;
        int m_category;

        bool operator<(const RegistryId& r) const;
    };

    constexpr const char* c_szAssetPaths[] =
    {
        "platform/assets/ConveyorStraight",

        "platform/assets/ConveyorCornerClockwise",
        "platform/assets/ConveyorCornerAntiClockwise",

        "platform/assets/Player",
        "platform/assets/Junction",
        "platform/assets/Tunnel",
        "platform/assets/Storage",
    };

    constexpr const char* c_szDataPaths[] =
    {
        "maps/circle",
        "maps/simple",
        "data/producers/MetalGenerator",
    };

    constexpr const char* c_szItemsPaths[] =
    {
        "data/items/IronOre",
        "data/items/CopperOre",
        "data/items/Coal",
    };

    constexpr const char* c_szItemAssetsPaths[] =
    {
        "platform/assets/items/IronOre",
        "platform/assets/items/CopperOre",
        "platform/assets/items/Coal",
    };

    constexpr const char* c_szFactoryPaths[] =
    {
        "data/Factories/CopperMine",
        "data/Factories/CopperSmelter",
    };

    constexpr const char* c_szFactoryAssetsPaths[] =
    {
        "platform/assets/Factories/CopperMine",
        "platform/assets/Factories/CopperSmelter",
    };

    constexpr const char* c_szInserterPaths[] =
    {
        "data/inserters/Basic",
    };

    constexpr const char* c_szInserterAssetsPaths[] =
    {
        "platform/assets/inserters/Basic",
    };

    constexpr const char* c_szRecipes[] =
    {
        "data/recipes/CopperOre",
        "data/recipes/CopperPlate",
    };
     
    constexpr const char* const* c_szCategoryPaths[] =
    {
        c_szAssetPaths,
        c_szDataPaths,
        c_szItemsPaths,
        c_szItemAssetsPaths,
        c_szFactoryPaths,
        c_szFactoryAssetsPaths,
        c_szInserterPaths,
        c_szInserterAssetsPaths,
        c_szRecipes,
    };

    namespace visual
    {
        constexpr RegistryId Conveyor_Straight = { 0, 0 };
        constexpr RegistryId Conveyor_CornerClockwise = { 1, 0 };
        constexpr RegistryId Conveyor_CornerAntiClockwise = { 2, 0 };

        constexpr RegistryId Player = { 3, 0 };
        constexpr RegistryId Junction = { 4, 0 };
        constexpr RegistryId Tunnel = { 5, 0 };
        constexpr RegistryId Storage = { 6, 0 };

        constexpr RegistryId IronOre = { 0, 2 };
        constexpr RegistryId CopperOre = { 1, 2 };
        constexpr RegistryId Coal = { 2, 2 };
    }

    namespace data
    {
        constexpr RegistryId MapCircle = { 0, 1 };
        constexpr RegistryId MapSimple = { 1, 1 };
        constexpr RegistryId Generator_Metal = { 2, 1 };
    }
}
