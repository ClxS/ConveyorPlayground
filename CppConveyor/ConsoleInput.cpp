#include "Input.h"
#include <Windows.h>

#define VK_V 0x0056
#define VK_7 0x0036

void cpp_conv::input::receiveInput(std::queue<cpp_conv::commands::InputCommand>& commands)
{
	DWORD dwPendingEvents;

	HANDLE hInputHandle = GetStdHandle(STD_INPUT_HANDLE);
	if (!GetNumberOfConsoleInputEvents(hInputHandle, &dwPendingEvents))
	{
		printf("GetNumberOfConsoleInputEvents failed: %i\n", GetLastError());
		return;
	}

	while (dwPendingEvents > 0)
	{
		DWORD dwEventsRecieved;
		INPUT_RECORD recordBuffer[32];
		if (!ReadConsoleInput(hInputHandle, recordBuffer, sizeof(recordBuffer) / sizeof(INPUT_RECORD), &dwEventsRecieved))
		{
			printf("ReadConsoleInput failed: %i\n", GetLastError());
			return;
		}

		if (dwEventsRecieved > dwPendingEvents)
		{
			dwPendingEvents = 0;
		}
		else
		{
			dwPendingEvents -= dwEventsRecieved;
		}

		for (DWORD i = 0; i < dwEventsRecieved; ++i)
		{
			switch (recordBuffer[i].EventType)
			{
			case KEY_EVENT:
				switch (recordBuffer[i].Event.KeyEvent.wVirtualKeyCode)
				{
				case VK_UP: commands.push(cpp_conv::commands::InputCommand::MoveDown); break;
				case VK_DOWN: commands.push(cpp_conv::commands::InputCommand::MoveUp); break;
				case VK_LEFT: commands.push(cpp_conv::commands::InputCommand::MoveLeft); break;
				case VK_RIGHT: commands.push(cpp_conv::commands::InputCommand::MoveRight); break;
				case VK_7: commands.push(cpp_conv::commands::InputCommand::PlaceConveyorDown); break;
				case VK_V: commands.push(cpp_conv::commands::InputCommand::PlaceConveyorUp); break;
				case VK_OEM_COMMA: commands.push(cpp_conv::commands::InputCommand::PlaceConveyorLeft); break;
				case VK_OEM_PERIOD: commands.push(cpp_conv::commands::InputCommand::PlaceConveyorRight); break;
				}
				break;
			}
		}
	}
}
