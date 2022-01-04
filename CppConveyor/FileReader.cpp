#include "FileReader.h"

#include <fstream>
#include "Conveyor.h"
#include "Producer.h"

void cpp_conv::file_reader::readFile(std::string strFileName, cpp_conv::grid::EntityGrid& grid, std::vector<cpp_conv::Conveyor*>& vConveyors, std::vector<cpp_conv::Producer*>& vProducers)
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
            case '/': pEntity = new cpp_conv::Conveyor(col, row, Direction::Up); break;
            case 'A':
            {
                cpp_conv::Producer* pProducer = new cpp_conv::Producer(col, row, Direction::Right, new cpp_conv::Copper(), 5);
                vProducers.push_back(pProducer);
                pEntity = pProducer;
                break;
            }
            case 'D':
            {
                cpp_conv::Producer* pProducer = new cpp_conv::Producer(col, row, Direction::Left, new cpp_conv::Copper(), 5);
                vProducers.push_back(pProducer);
                pEntity = pProducer;
                break;
            }
            }

            if (pEntity)
            {
                grid[row][col] = pEntity;

                if (pEntity->m_eEntityKind == EntityKind::Conveyor)
                {
                    vConveyors.push_back(reinterpret_cast<cpp_conv::Conveyor*>(pEntity));
                }
            }
        }

        row++;
    }

    file.close();
}
