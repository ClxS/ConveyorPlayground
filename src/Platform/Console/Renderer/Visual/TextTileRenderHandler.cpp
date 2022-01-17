#include "TextTileRenderHandler.h"
#include "RenderContext.h"
#include "RenderableAsset.h"
#include "Transform2D.h"
#include "Renderer.h"
#include "TextTileAsset.h"
#include "ConsoleWriteUtility.h"
#include "SelfRegistration.h"
#include <utility>

#undef max

wchar_t rotateCharIfAllowed(wchar_t input, int cycle)
{
    constexpr const wchar_t* borders = L"│─";
    constexpr const wchar_t* arrows = L"→↓←↑";

    switch (input)
    {
    case L'→': return arrows[(cycle + 0) % 4];
    case L'↓': return arrows[(cycle + 1) % 4];
    case L'←': return arrows[(cycle + 2) % 4];
    case L'↑': return arrows[(cycle + 3) % 4];
    case L'│': return borders[(cycle + 0) % 2];
    case L'─': return borders[(cycle + 1) % 2];
    default: return input;
    }
}

void tileRenderer(cpp_conv::RenderContext& kContext, const cpp_conv::resources::RenderableAsset* pAsset, const cpp_conv::Transform2D& kTransform, cpp_conv::Colour kColourOverride)
{
    const cpp_conv::resources::TextTileAsset* pTile = reinterpret_cast<const cpp_conv::resources::TextTileAsset*>(pAsset);
    const std::vector<std::wstring>* vData = &pTile->GetData();

    int maxWidth = 0;
    for (int iRow = 0; iRow < (*vData).size(); ++iRow)
    {
        maxWidth = std::max(maxWidth, (int)((*vData)[iRow].size()));
    }

    for (int iRow = 0; iRow < (*vData).size(); ++iRow)
    {
        for (int iCol = 0; iCol < (*vData)[iRow].size(); ++iCol)
        {
            if ((*vData)[iRow][iCol] == ' ')
            {
                continue;
            }

            switch (kTransform.m_rotation)
            {
            case Rotation::DegZero:
                cpp_conv::renderer::setCell(kContext, (*vData)[iRow][iCol], kTransform.GetX() + iCol, kTransform.GetY() + iRow, cpp_conv::renderer::getWin32Colour(kColourOverride));
                break;
            case Rotation::Deg90:
                cpp_conv::renderer::setCell(kContext, rotateCharIfAllowed((*vData)[iRow][iCol], 1), kTransform.GetX() + ((int)(*vData).size() - 1 - iRow), kTransform.GetY() + iCol, cpp_conv::renderer::getWin32Colour(kColourOverride));
                break;
            case Rotation::Deg180:
                cpp_conv::renderer::setCell(kContext, rotateCharIfAllowed((*vData)[iRow][iCol], 2), kTransform.GetX() + (maxWidth - 1 - iCol), kTransform.GetY() + ((int)(*vData).size() - 1 - iRow), cpp_conv::renderer::getWin32Colour(kColourOverride));
                break;
            case Rotation::Deg270:
                cpp_conv::renderer::setCell(kContext, rotateCharIfAllowed((*vData)[iRow][iCol], 3), kTransform.GetX() + iRow, kTransform.GetY() + (maxWidth - 1 - iCol), cpp_conv::renderer::getWin32Colour(kColourOverride));
                break;
            }
        } 
    }
}

REGISTER_RENDER_HANDLER(cpp_conv::resources::TextTileAsset, tileRenderer);
