#pragma once

#include <string>
#include "ResourceAsset.h"

namespace cpp_conv
{
    class ItemDefinition : public cpp_conv::resources::ResourceAsset
    {
    public:
        ItemDefinition(uint64_t internalId, std::string strName, wchar_t displayIcon)
            : m_internalId(internalId)
            , m_strName(std::move(strName))
            , m_displayIcon(displayIcon)
        {
        }

        uint64_t GetInternalId() const { return m_internalId; }

        const std::string& GetName() const { return m_strName; }
        wchar_t GetDisplayIcon() const { return m_displayIcon; }

    private:
        uint64_t m_internalId;
        std::string m_internalName;
        std::string m_strName;
        wchar_t m_displayIcon;
    };
}
