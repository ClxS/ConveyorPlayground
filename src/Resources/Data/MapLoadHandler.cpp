#include "MapLoadHandler.h"
#include "ResourceManager.h"
#include "Map.h"
#include "Entity.h"
#include "Conveyor.h"
#include "Junction.h"
#include "Producer.h"
#include "Underground.h"
#include "Storage.h"

#include <sstream>
#include <iostream>

cpp_conv::resources::ResourceAsset* mapLoadHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const char* pStrData = reinterpret_cast<const char*>(rData.m_pData);

    std::istringstream ss(pStrData);
    std::string strLine;

    auto pMap = new cpp_conv::resources::Map();
    int iRow = 0;
    while (std::getline(ss, strLine))
    {
        for (int iCol = 0; iCol < strLine.size(); iCol++)
        {
            cpp_conv::Entity* pEntity = nullptr;
            switch (strLine[iCol])
            {
            case '>': pEntity = new cpp_conv::Conveyor(iCol, iRow, Direction::Right); break;
            case '<': pEntity = new cpp_conv::Conveyor(iCol, iRow, Direction::Left); break;
            case '^': pEntity = new cpp_conv::Conveyor(iCol, iRow, Direction::Down); break;
            case 'v': pEntity = new cpp_conv::Conveyor(iCol, iRow, Direction::Up); break;
            case 'A': pEntity = new cpp_conv::Producer(iCol, iRow, Direction::Right, new cpp_conv::Copper(), 7); break;
            case 'D': pEntity = new cpp_conv::Producer(iCol, iRow, Direction::Left, new cpp_conv::Copper(), 7); break;
            case 'F': pEntity = new cpp_conv::Producer(iCol, iRow, Direction::Down, new cpp_conv::Copper(), 7); break;
            case 'G': pEntity = new cpp_conv::Producer(iCol, iRow, Direction::Up, new cpp_conv::Copper(), 7); break;
            case 'J': pEntity = new cpp_conv::Junction(iCol, iRow); break;
            case 'S': pEntity = new cpp_conv::Storage(iCol, iRow); break;
            case 'u': pEntity = new cpp_conv::Underground(iCol, iRow, Direction::Down); break;
            case 'y': pEntity = new cpp_conv::Underground(iCol, iRow, Direction::Up); break;
            case 'i': pEntity = new cpp_conv::Underground(iCol, iRow, Direction::Left); break;
            case 'o': pEntity = new cpp_conv::Underground(iCol, iRow, Direction::Right); break;
            }

            if (pEntity)
            {
                pMap->GetGrid()[iRow][iCol] = pEntity;

                if (pEntity->m_eEntityKind == EntityKind::Conveyor)
                {
                    pMap->GetConveyors().push_back(reinterpret_cast<cpp_conv::Conveyor*>(pEntity));
                }
                else
                {
                    pMap->GetOtherEntities().push_back(pEntity);
                }
            }
        }

        iRow++;
    }

    return pMap;
}

void cpp_conv::resources::registerMapLoadHandler()
{
    cpp_conv::resources::resource_manager::registerTypeHandler<cpp_conv::resources::Map>(&mapLoadHandler);
}