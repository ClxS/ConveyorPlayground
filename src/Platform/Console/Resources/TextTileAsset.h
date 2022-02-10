#pragma once

#include "RenderableAsset.h"
#include <vector>
#include <string>

#if !defined(_CONSOLE)
#error Invalid Include
#endif

namespace cpp_conv::resources
{
    class TextTileAsset : public RenderableAsset
    {
    public:
        TextTileAsset(std::vector<std::wstring> vChars)
            : m_vChars(std::move(vChars))
        {
        }

        [[nodiscard]] const std::vector<std::wstring>& GetData() const { return m_vChars; }
    private:
        std::vector<std::wstring> m_vChars;
    };

    using TileAsset = TextTileAsset;
}
