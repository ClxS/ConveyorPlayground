#pragma once

namespace cpp_conv::resources
{
	class ResourceAsset;

	namespace resource_manager
	{
		struct FileData;
	}

	void registerMapLoadHandler();

	ResourceAsset* mapLoadHandler(cpp_conv::resources::resource_manager::FileData& rData);
}