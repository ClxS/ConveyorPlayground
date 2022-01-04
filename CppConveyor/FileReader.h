#pragma once

#include <string>
#include <vector>

#include "Grid.h"

namespace cpp_conv
{
	class Conveyor;
	class Producer;
}

namespace cpp_conv::file_reader
{
	void readFile(std::string strFileName, cpp_conv::grid::EntityGrid& grid, std::vector<cpp_conv::Conveyor*>& vConveyors, std::vector<cpp_conv::Producer*>& vProducers);
}