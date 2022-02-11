#include "SelfRegistration.h"
#include "Renderer.h"

std::vector<cpp_conv::resources::registration::AssetLoadHandler*>& getAssetLoadStore()
{
    static std::vector<cpp_conv::resources::registration::AssetLoadHandler*> s_handlers;
    return s_handlers;
}

std::vector<cpp_conv::resources::registration::AssetRenderHandler*>& getAssetRenderStore()
{
    static std::vector<cpp_conv::resources::registration::AssetRenderHandler*> s_handlers;
    return s_handlers;
}

std::vector<cpp_conv::resources::registration::LoadHandler*>& getLoadStore()
{
    static std::vector<cpp_conv::resources::registration::LoadHandler*> s_handlers;
    return s_handlers;
}

void cpp_conv::resources::registration::processSelfRegistrations()
{
    for (const auto pHandler : getAssetLoadStore())
    {
        registerTypeHandler(pHandler->m_rType, pHandler->m_rFunc);
    }

    for (const auto pHandler : getAssetRenderStore())
    {
        renderer::registerTypeHandler(pHandler->m_rType, pHandler->m_rFunc);
    }

    for (const auto pHandler : getLoadStore())
    {
        pHandler->m_rFunc();
    }
}

void cpp_conv::resources::registration::registerAssetLoadHandler(AssetLoadHandler* pHandler)
{
    getAssetLoadStore().push_back(pHandler);
}

void cpp_conv::resources::registration::registerAssetRenderHandler(AssetRenderHandler* pHandler)
{
    getAssetRenderStore().push_back(pHandler);
}

void cpp_conv::resources::registration::registerLoadHandler(struct LoadHandler* pHandler)
{
    getLoadStore().push_back(pHandler);
}
