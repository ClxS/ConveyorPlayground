#include "TextTileRenderHandler.h"
#include "RenderContext.h"
#include "RenderableAsset.h"
#include "Transform2D.h"
#include "Renderer.h"
#include "TextTileAsset.h"
#include "ConsoleWriteUtility.h"
#include <utility>

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
		maxWidth = max(maxWidth, (int)((*vData)[iRow].size()));
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
			case cpp_conv::Transform2D::Rotation::DegZero:
				cpp_conv::renderer::setCell(kContext, (*vData)[iRow][iCol], kTransform.m_x + iCol, kTransform.m_y + iRow, cpp_conv::renderer::getWin32Colour(kColourOverride));
				break;
			case cpp_conv::Transform2D::Rotation::Deg90:
				cpp_conv::renderer::setCell(kContext, rotateCharIfAllowed((*vData)[iRow][iCol], 1), kTransform.m_x + ((*vData).size() - 1 - iRow), kTransform.m_y + iCol, cpp_conv::renderer::getWin32Colour(kColourOverride));
				break;
			case cpp_conv::Transform2D::Rotation::Deg180:
				cpp_conv::renderer::setCell(kContext, rotateCharIfAllowed((*vData)[iRow][iCol], 2), kTransform.m_x + (maxWidth - 1 - iCol), kTransform.m_y + ((*vData).size() - 1 - iRow), cpp_conv::renderer::getWin32Colour(kColourOverride));
				break;
			case cpp_conv::Transform2D::Rotation::Deg270:
				cpp_conv::renderer::setCell(kContext, rotateCharIfAllowed((*vData)[iRow][iCol], 3), kTransform.m_x + iRow, kTransform.m_y + (maxWidth - 1 - iCol), cpp_conv::renderer::getWin32Colour(kColourOverride));
				break;
			}
		} 
	}
}

void cpp_conv::renderer::registerTileRenderHandler()
{
	cpp_conv::renderer::registerTypeHandler<cpp_conv::resources::TextTileAsset>(&tileRenderer);
}
