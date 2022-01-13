#pragma once
#include <filesystem>

namespace cpp_conv::resources::registry
{
    struct PathExtension
    {
        PathExtension(const char* path)
            : m_Path(path)
        {
        }

        std::filesystem::path m_Path;
    };

    inline static std::filesystem::path operator+(const std::filesystem::path& path, const PathExtension& extension)
    {
        auto copy = path;
        return copy.replace_extension(extension.m_Path);
    }

    #define P(X, EXT) std::filesystem::path(X ".ext") + EXT

    inline static const PathExtension c_dataExtension = ".txt";
    #if defined(_CONSOLE)
    inline static const PathExtension c_assetExtension = ".txt";
    #elif defined(_SDL)
    inline static const PathExtension c_assetExtension = ".bmp";
    #endif

    struct RegistryId
    {
        int m_index;
        int m_category;

        bool operator<(const RegistryId& r) const;
    };

    inline static const std::filesystem::path c_szAssetPaths[] =
    {
        P("platform/assets/ConveyorStraight", c_assetExtension),

        P("platform/assets/ConveyorCornerClockwise", c_assetExtension),
        P("platform/assets/ConveyorCornerAntiClockwise", c_assetExtension),

        P("platform/assets/Player", c_assetExtension),
        P("platform/assets/Junction", c_assetExtension),
        P("platform/assets/Tunnel", c_assetExtension),
        P("platform/assets/Storage", c_assetExtension),
        P("platform/assets/StairsUp", c_assetExtension),
        P("platform/assets/StairsDown", c_assetExtension),
    };

    inline static const std::filesystem::path c_szDataPaths[] =
    {
        P("maps/circle", c_dataExtension),
        P("maps/simple", c_dataExtension),
        P("maps/bigmap", c_dataExtension),
    };

    inline static const std::filesystem::path c_szItemsPaths[] =
    {
        P("data/items/IronOre", c_dataExtension),
        P("data/items/CopperOre", c_dataExtension),
        P("data/items/Coal", c_dataExtension),
        P("data/items/CopperPlate", c_dataExtension),
    };

    inline static const std::filesystem::path c_szItemAssetsPaths[] =
    {
        P("platform/assets/items/IronOre", c_assetExtension),
        P("platform/assets/items/CopperOre", c_assetExtension),
        P("platform/assets/items/Coal", c_assetExtension),
        P("platform/assets/items/CopperPlate", c_assetExtension),
    };

    inline static const std::filesystem::path c_szFactoryPaths[] =
    {
        P("data/Factories/CopperMine", c_dataExtension),
        P("data/Factories/CopperSmelter", c_dataExtension),
    };

    inline static const std::filesystem::path c_szFactoryAssetsPaths[] =
    {
        P("platform/assets/Factories/CopperMine", c_assetExtension),
        P("platform/assets/Factories/CopperSmelter", c_assetExtension),
    };

    inline static const std::filesystem::path c_szInserterPaths[] =
    {
        P("data/inserters/Basic", c_dataExtension),
    };

    inline static const std::filesystem::path c_szInserterAssetsPaths[] =
    {
        P("platform/assets/inserters/Basic", c_assetExtension),
    };

    inline static const std::filesystem::path c_szRecipes[] =
    {
        P("data/recipes/CopperOre", c_dataExtension),
        P("data/recipes/CopperPlate", c_dataExtension),
    };

    inline static const std::filesystem::path c_szConveyors[] =
    {
        P("data/conveyors/Basic", c_dataExtension),
    };
     
    inline static const std::filesystem::path* c_szCategoryPaths[] =
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
        c_szConveyors
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
        constexpr RegistryId StairsUp = { 7, 0 };
        constexpr RegistryId StairsDown = { 8, 0 };

        constexpr RegistryId IronOre = { 0, 2 };
        constexpr RegistryId CopperOre = { 1, 2 };
        constexpr RegistryId Coal = { 2, 2 };
    }

    namespace data
    {
        constexpr RegistryId MapCircle = { 0, 1 };
        constexpr RegistryId MapSimple = { 1, 1 };
        constexpr RegistryId MapBig = { 2, 1 };
    }
}
