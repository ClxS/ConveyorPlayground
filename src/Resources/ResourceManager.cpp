#include "ResourceManager.h"
#include "ResourceAsset.h"

#include "MapLoadHandler.h"

#include <map>
#include <memory>
#include <mutex>
#include <fstream>

using FileData = cpp_conv::resources::resource_manager::FileData;
using RegistryId = cpp_conv::resources::resource_registry::RegistryId;
using WeakResourcePtr = std::weak_ptr<cpp_conv::resources::ResourceAsset>;
using TypeId = size_t;
using LoadedAssetContainer = std::map<RegistryId, WeakResourcePtr>;

static std::map<TypeId, std::function<cpp_conv::resources::ResourceAsset* (FileData&)>*> g_typeHandlers;
static std::map<TypeId, LoadedAssetContainer*> g_loadedTypes = {};

std::mutex& getStateLock()
{
	static std::mutex s_stateLock;
	return s_stateLock;
}

// TODO[CJones] Add a persistence mechanism which keeps loaded assets in memory for X amount of time. This will let us dynamically load resources instead of
// storing them in our game objects, but also prevent constant load/unloads every frame. This store should also have a flush mechanism to clear it on in cases like
// scene transitions
std::function<cpp_conv::resources::ResourceAsset* (FileData&)>* getTypeHandler(const std::type_info& type)
{
	// No need to lock here, this is only called in the context of an existing lock
	auto iter = g_typeHandlers.find(type.hash_code());
	if (iter == g_typeHandlers.end())
	{
		return nullptr;
	}

	return iter->second;
}

void cpp_conv::resources::resource_manager::initialize()
{
	cpp_conv::resources::registerMapLoadHandler();
}

void cpp_conv::resources::resource_manager::registerTypeHandler(const std::type_info& type, std::function<cpp_conv::resources::ResourceAsset*(FileData&)> fHandler)
{
	std::lock_guard<std::mutex> lock(getStateLock());
	g_typeHandlers[type.hash_code()] = new std::function<cpp_conv::resources::ResourceAsset* (FileData&)>(fHandler);
	g_loadedTypes[type.hash_code()] = new LoadedAssetContainer();
}

cpp_conv::resources::resource_manager::FileData getFileData(cpp_conv::resources::resource_registry::RegistryId kAssetId)
{
	constexpr const char* c_fileExt = ".txt";
	std::string path = "data/";
	path += cpp_conv::resources::resource_registry::c_szCategoryPaths[kAssetId.m_category][kAssetId.m_index];
	path += c_fileExt;

	std::fstream file;
	file.open(path);
	if (file.fail())
	{
		return {};
	}

	file.seekg(0, std::ios::end);
	size_t uiLength = file.tellg();
	file.seekg(0, std::ios::beg);

	if (uiLength == 0)
	{
		return {};
	}

	uint8_t* pData = new uint8_t[uiLength];
	file.read(reinterpret_cast<char*>(pData), uiLength);

	return { pData, uiLength };
}

cpp_conv::resources::AssetPtr<cpp_conv::resources::ResourceAsset> cpp_conv::resources::resource_manager::loadAsset(const std::type_info& type, cpp_conv::resources::resource_registry::RegistryId kAssetId)
{
	std::lock_guard<std::mutex> lock(getStateLock());
	auto loadedContainerIter = g_loadedTypes.find(type.hash_code());
	if (loadedContainerIter == g_loadedTypes.end())
	{
		return nullptr;
	}

	auto existingAssetEntry = loadedContainerIter->second->find(kAssetId);
	if (existingAssetEntry != loadedContainerIter->second->end())
	{
		AssetPtr<ResourceAsset> pExistingAsset = existingAssetEntry->second.lock();
		if (pExistingAsset)
		{
			return pExistingAsset;
		}
		else
		{
			// Asset no longer exists, we need to re-load it.
			loadedContainerIter->second->erase(existingAssetEntry);
		}
	}

	std::function<ResourceAsset* (FileData&)>* fHandler = getTypeHandler(type);
	if (!fHandler || !(*fHandler))
	{
		return nullptr;
	}

	FileData kFileData = getFileData(kAssetId);
	if (!kFileData.m_pData)
	{
		return nullptr;
	}

	ResourceAsset* pAsset = (*fHandler)(kFileData);
	
	delete[] kFileData.m_pData;

	if (!pAsset)
	{
		return nullptr;
	}

	cpp_conv::resources::AssetPtr<cpp_conv::resources::ResourceAsset> pSharedAsset(pAsset);
	(*loadedContainerIter->second)[kAssetId] = pSharedAsset;
	return pSharedAsset;
}
