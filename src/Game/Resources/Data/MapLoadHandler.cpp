#include "MapLoadHandler.h"

#include <cassert>

#include "Conveyor.h"
#include "Entity.h"
#include "Factory.h"
#include "ItemRegistry.h"
#include "Junction.h"
#include "Map.h"
#include "Storage.h"
#include "Tunnel.h"

#include <iostream>
#include <sstream>
#include "Inserter.h"
#include "LaunchPad.h"
#include "Stairs.h"
#include "AtlasResource/FileData.h"

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> cpp_conv::resources::mapAssetHandler(const atlas::resource::FileData& rData)
{
    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData.get());

    const auto pTruncatedData = std::string_view(pStrData, static_cast<uint32_t>(rData.m_Size));
    auto ss = std::istringstream(std::string(pTruncatedData));

    std::vector<std::string> grid;

    atlas::resource::AssetPtr<Map> pMap {new Map()};

    const Eigen::Vector3i size1X1 = {1, 1, 1};
    std::string strLine;
    while (std::getline(ss, strLine))
    {
        grid.push_back(strLine);
    }

    for(int32_t iRow = 0; iRow < grid.size(); iRow++)
    {
        for (size_t iCol = 0; iCol < grid[iRow].size(); iCol++)
        {
            Entity* pEntity = nullptr;
            switch (grid[iRow][iCol])
            {
            case '>': pEntity = new
                    Conveyor({static_cast<int32_t>(iCol), 0, iRow}, size1X1, Direction::Right);
                break;
            case '<': pEntity = new Conveyor({static_cast<int32_t>(iCol), 0, iRow}, size1X1, Direction::Left);
                break;
            case '^': pEntity = new Conveyor({static_cast<int32_t>(iCol), 0, iRow}, size1X1, Direction::Down);
                break;
            case 'v': pEntity = new Conveyor({static_cast<int32_t>(iCol), 0, iRow}, size1X1, Direction::Up);
                break;
            case 'I': pEntity = new Inserter({static_cast<int32_t>(iCol), 0, iRow}, size1X1, Direction::Down,
                                             InserterId::FromStringId("INSERTER_BASIC"));
                break;
            case 'U': pEntity = new Inserter({static_cast<int32_t>(iCol), 0, iRow}, size1X1, Direction::Up,
                                             InserterId::FromStringId("INSERTER_BASIC"));
                break;
            case 'T': pEntity = new Inserter({static_cast<int32_t>(iCol), 0, iRow}, size1X1, Direction::Left,
                                             InserterId::FromStringId("INSERTER_BASIC"));
                break;
            case 'Y': pEntity = new Inserter({static_cast<int32_t>(iCol), 0, iRow}, size1X1, Direction::Right,
                                             InserterId::FromStringId("INSERTER_BASIC"));
                break;
            case 'A': pEntity = new Factory({static_cast<int32_t>(iCol + 1), 0, iRow + 1}, Direction::Right,
                                            FactoryId::FromStringId("FACTORY_COPPER_MINE"));
                break;
            case 'D': pEntity = new Factory({static_cast<int32_t>(iCol + 1), 0, iRow + 1}, Direction::Left,
                                            FactoryId::FromStringId("FACTORY_COPPER_MINE"));
                break;
            case 'F': pEntity = new Factory({static_cast<int32_t>(iCol + 1), 0, iRow + 1}, Direction::Down,
                                            FactoryId::FromStringId("FACTORY_COPPER_MINE"));
                break;
            case 'G': pEntity = new Factory({static_cast<int32_t>(iCol + 1), 0, iRow + 1}, Direction::Up,
                                            FactoryId::FromStringId("FACTORY_COPPER_MINE"));
                break;
            case 'C': pEntity = new Factory({static_cast<int32_t>(iCol + 1), 0, iRow + 1}, Direction::Right,
                                            FactoryId::FromStringId("FACTORY_COPPER_SMELTER"));
                break;
            case 'J': pEntity = new Junction({static_cast<int32_t>(iCol), 0, iRow}, size1X1);
                break;
            case 'S': pEntity = new Storage({static_cast<int32_t>(iCol), 0, iRow}, size1X1, 16, 256);
                break;
            case '@': pEntity = new Stairs({static_cast<int32_t>(iCol), 0, iRow}, {1, 1, 2}, Direction::Right,
                                           true);
                break;
            case 'u': pEntity = new Tunnel({static_cast<int32_t>(iCol), 0, iRow}, size1X1, Direction::Down);
                break;
            case 'y': pEntity = new Tunnel({static_cast<int32_t>(iCol), 0, iRow}, size1X1, Direction::Up);
                break;
            case 'i': pEntity = new Tunnel({static_cast<int32_t>(iCol), 0, iRow}, size1X1, Direction::Left);
                break;
            case 'o': pEntity = new Tunnel({static_cast<int32_t>(iCol), 0, iRow}, size1X1, Direction::Right);
                break;
            case 'L': pEntity = new LaunchPad({static_cast<int32_t>(iCol + 5), 0, iRow + 5}, Direction::Right);
            default: ; // Ignored
            }

            if (pEntity)
            {
                for(size_t i = iRow; i < iRow + pEntity->m_size.x(); i++)
                {
                    for(size_t j = iCol; j < iCol + pEntity->m_size.z(); j++)
                    {
                        grid[i][j] = ' ';
                    }
                }

                if (pEntity->m_eEntityKind == EntityKind::Conveyor)
                {
                    pMap->GetConveyors().push_back(static_cast<Conveyor*>(pEntity));
                }
                else
                {
                    pMap->GetOtherEntities().push_back(pEntity);
                }
            }
        }
    }

    return pMap;
}
