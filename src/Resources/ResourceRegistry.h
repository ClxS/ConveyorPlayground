#pragma once

namespace cpp_conv::resources::registry
{
	struct RegistryId
	{
		int m_index;
		int m_category;

		bool operator<(const RegistryId& r) const
		{
			return (m_category < r.m_category || (m_index == r.m_index && m_category < r.m_category));
		}
	};

	constexpr const char* c_szAssetPaths[] =
	{
		"platform/assets/ConveyorStraight",

		"platform/assets/ConveyorCornerClockwise",
		"platform/assets/ConveyorCornerAntiClockwise",

		"platform/assets/Player",
		"platform/assets/Junction",
		"platform/assets/Tunnel",
	};

	constexpr const char* c_szItemsPaths[] =
	{
		"platform/assets/items/IronOre",
		"platform/assets/items/CopperOre",
		"platform/assets/items/Coal",
	};

	constexpr const char* c_szDataPaths[] =
	{
		"maps/circle",
		"maps/simple",
		"data/producers/MetalGenerator",
	};

	constexpr const char* const* c_szCategoryPaths[] =
	{
		c_szAssetPaths,
		c_szDataPaths,
		c_szItemsPaths
	};

	namespace visual
	{
		constexpr RegistryId Conveyor_Straight = { 0, 0 };
		constexpr RegistryId Conveyor_CornerClockwise = { 1, 0 };
		constexpr RegistryId Conveyor_CornerAntiClockwise = { 2, 0 };

		constexpr RegistryId Player = { 3, 0 };
		constexpr RegistryId Junction = { 4, 0 };
		constexpr RegistryId Tunnel = { 5, 0 };

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