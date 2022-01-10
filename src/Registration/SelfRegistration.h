#pragma once

#include <functional>
#include <memory>
#include "ResourceAsset.h"
#include "ResourceManager.h"

namespace cpp_conv::resources::registration
{
    using TypeInfo = std::type_info;
    using AssetLoadFunc = std::function<ResourceAsset* (cpp_conv::resources::resource_manager::FileData&)>;
    using LoadFunc = std::function<void(void)>;

    void processSelfRegistrations();
    void registerAssetLoadHandler(struct AssetLoadHandler* pHandler);
    void registerLoadHandler(struct LoadHandler* pHandler);

    struct AssetLoadHandler
    {
        AssetLoadHandler(const TypeInfo& type, AssetLoadFunc func)
            : m_rType(type)
            , m_rFunc(func)
        {
            registerAssetLoadHandler(this);
        }

        const TypeInfo& m_rType;
        AssetLoadFunc m_rFunc;
    };

    struct LoadHandler
    {
        LoadHandler(LoadFunc func)
            : m_rFunc(func)
        {
            registerLoadHandler(this);
        }

        LoadFunc m_rFunc;
    };
}

#define REGISTER_ASSET_LOAD_HANDLER(TYPE, HANDLER)\
    namespace {\
        static cpp_conv::resources::registration::AssetLoadHandler g_##__LINE__##AssetLoadHandler(typeid(TYPE), &HANDLER);\
    }

#define REGISTER_LOAD_HANDLER(HANDLER)\
    namespace {\
        static cpp_conv::resources::registration::LoadHandler g_##__LINE__##LoadHandler(&HANDLER);\
    }