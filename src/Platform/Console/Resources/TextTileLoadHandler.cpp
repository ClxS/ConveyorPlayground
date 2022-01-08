#include "TextTileLoadHandler.h"
#include "ResourceAsset.h"
#include "ResourceManager.h"
#include "TextTileAsset.h"

#include <sstream>
#include "SelfRegistration.h"

cpp_conv::resources::ResourceAsset* textTileLoadHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const wchar_t* pStrData = reinterpret_cast<const wchar_t*>(rData.m_pData);
     
    std::wstring copy(pStrData, rData.m_uiSize / sizeof(wchar_t));
    std::wistringstream ss(copy);
    std::wstring strLine;

    std::vector<std::wstring> txt;
    while (std::getline(ss, strLine))
    {
        txt.push_back(strLine);
    }
     
    auto pTile = new cpp_conv::resources::TextTileAsset(txt);
    return pTile;
}

REGISTER_ASSET_LOAD_HANDLER(cpp_conv::resources::TextTileAsset, textTileLoadHandler);
