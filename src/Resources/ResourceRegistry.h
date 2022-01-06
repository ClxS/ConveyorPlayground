#pragma once

namespace cpp_conv::resources::resource_registry
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
		"platform/assets/ConveyorStraightUp",
		"platform/assets/ConveyorStraightDown",
		"platform/assets/ConveyorStraightLeft",
		"platform/assets/ConveyorStraightRight",

		"platform/assets/ConveyorCornerClockwise",
		"platform/assets/ConveyorCornerAntiClockwise",
	};

	constexpr const char* c_szDataPaths[] =
	{
		"maps/circle",
		"data/producers/MetalGenerator",
	};

	constexpr const char* const* c_szCategoryPaths[] =
	{
		c_szAssetPaths,
		c_szDataPaths,
	};

	namespace asset_assets
	{
		constexpr RegistryId Conveyor_StraightUp = { 0, 0 };
		constexpr RegistryId Conveyor_StraightDown = { 1, 0 };
		constexpr RegistryId Conveyor_StraightLeft = { 2, 0 };
		constexpr RegistryId Conveyor_StraightRight = { 3, 0 };
		constexpr RegistryId Conveyor_CornerClockwise = { 4, 0 };
		constexpr RegistryId Conveyor_CornerAntiClockwise = { 5, 0 };
	}

	namespace data_assets
	{
		constexpr RegistryId MapCircle = { 0, 1 };
		constexpr RegistryId Generator_Metal = { 1, 1 };
	}
}