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
	};

	namespace visual
	{
		constexpr RegistryId Conveyor_Straight = { 0, 0 };
		constexpr RegistryId Conveyor_CornerClockwise = { 1, 0 };
		constexpr RegistryId Conveyor_CornerAntiClockwise = { 2, 0 };
	}

	namespace data
	{
		constexpr RegistryId MapCircle = { 0, 1 };
		constexpr RegistryId MapSimple = { 1, 1 };
		constexpr RegistryId Generator_Metal = { 2, 1 };
	}
}