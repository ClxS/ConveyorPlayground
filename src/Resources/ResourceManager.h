#pragma once

#include <functional>
#include <memory>

#include "ResourceRegistry.h"
#include "AssetPtr.h"
#include "ResourceAsset.h"

namespace cpp_conv::resources::resource_manager
{
	struct FileData
	{
		uint8_t* m_pData;
		uint64_t m_uiSize;
	};

	void initialize();
	void registerTypeHandler(const std::type_info& type, std::function<ResourceAsset*(FileData&)> fHandler);

	AssetPtr<ResourceAsset> loadAsset(const std::type_info& type, resource_registry::RegistryId kAssetId);

	template<typename TType>
	void registerTypeHandler(std::function<ResourceAsset*(FileData& rData)> fHandler)
	{
		registerTypeHandler(typeid(TType), fHandler);
	}

	template<typename TType>
	AssetPtr<TType> loadAsset(resource_registry::RegistryId kAssetId)
	{
		auto pAsset = loadAsset(typeid(TType), kAssetId);
		if (!pAsset)
		{
			return nullptr;
		}

		return std::reinterpret_pointer_cast<TType>(pAsset);
	}
}