#include "FileReader.h"

#include <fstream>
#include "Conveyor.h"
#include "Producer.h"
#include "Junction.h"
#include "Underground.h"

void cpp_conv::file_reader::readFile(
    std::string strFileName, 
    cpp_conv::grid::EntityGrid& grid,
    std::vector<cpp_conv::Conveyor*>& vConveyors, 
    std::vector<cpp_conv::Entity*>& vOtherEntities
)
{
    std::ifstream file(strFileName.c_str());

    std::string buf;
    int row = 0;
    while (getline(file, buf))
    {
        for (int col = 0; col < buf.size(); col++)
        {
            cpp_conv::Entity* pEntity = nullptr;
            switch (buf[col])
            {
            case '>': pEntity = new cpp_conv::Conveyor(col, row, Direction::Right); break;
            case '<': pEntity = new cpp_conv::Conveyor(col, row, Direction::Left); break;
            case '^': pEntity = new cpp_conv::Conveyor(col, row, Direction::Down); break;
            case 'v': pEntity = new cpp_conv::Conveyor(col, row, Direction::Up); break;
            case 'A': pEntity = new cpp_conv::Producer(col, row, Direction::Right, new cpp_conv::Copper(), 7); break;
            case 'D': pEntity = new cpp_conv::Producer(col, row, Direction::Left, new cpp_conv::Copper(), 7); break;
            case 'F': pEntity = new cpp_conv::Producer(col, row, Direction::Down, new cpp_conv::Copper(), 7); break;
            case 'G': pEntity = new cpp_conv::Producer(col, row, Direction::Up, new cpp_conv::Copper(), 7); break;
            case 'J': pEntity = new cpp_conv::Junction(col, row); break;
            case 'u': pEntity = new cpp_conv::Underground(col, row, Direction::Down); break;
            case 'y': pEntity = new cpp_conv::Underground(col, row, Direction::Up); break;
            case 'i': pEntity = new cpp_conv::Underground(col, row, Direction::Left); break;
            case 'o': pEntity = new cpp_conv::Underground(col, row, Direction::Right); break;
            }

            if (pEntity)
            {
                grid[row][col] = pEntity;

                if (pEntity->m_eEntityKind == EntityKind::Conveyor)
                {
                    vConveyors.push_back(reinterpret_cast<cpp_conv::Conveyor*>(pEntity));
                }
                else
                {
                    vOtherEntities.push_back(pEntity);
                }
            }
        }

        row++;
    }

    file.close();
}
