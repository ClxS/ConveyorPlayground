#include "Input.h"
#include <Windows.h>
#include "Profiler.h"
#include "RenderContext.h"
#include "SwapChain.h"
#include "AppHost.h"
#include "SceneContext.h"

#define VK_V 0x0056

void cpp_conv::input::receiveInput(cpp_conv::SceneContext& kContext, cpp_conv::RenderContext& kRenderContext, std::queue<cpp_conv::commands::CommandType>& commands)
{
    DWORD dwPendingEvents;

    const HANDLE hInputHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (!GetNumberOfConsoleInputEvents(hInputHandle, &dwPendingEvents))
    {
        printf("GetNumberOfConsoleInputEvents failed: %i\n", GetLastError());
        return;
    }

    const DWORD fdwMode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (!SetConsoleMode(hInputHandle, fdwMode))
    {

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
                case '1': case VK_NUMPAD1: commands.push(cpp_conv::commands::CommandType::SelectItem1); break;
                case '2': case VK_NUMPAD2: commands.push(cpp_conv::commands::CommandType::SelectItem2); break;
                case '3': case VK_NUMPAD3: commands.push(cpp_conv::commands::CommandType::SelectItem3); break;
                case '4': case VK_NUMPAD4: commands.push(cpp_conv::commands::CommandType::SelectItem4); break;
                case '5': case VK_NUMPAD5: commands.push(cpp_conv::commands::CommandType::SelectItem5); break;
                case '6': case VK_NUMPAD6: commands.push(cpp_conv::commands::CommandType::SelectItem6); break;
                case '7': case VK_NUMPAD7: commands.push(cpp_conv::commands::CommandType::SelectItem7); break;
                case '8': case VK_NUMPAD8: commands.push(cpp_conv::commands::CommandType::SelectItem8); break;
                case '9': case VK_NUMPAD9: commands.push(cpp_conv::commands::CommandType::SelectItem9); break;
                case 'R': commands.push(cpp_conv::commands::CommandType::RotateSelection); break;
                case 'T': commands.push(cpp_conv::commands::CommandType::ToggleModifier); break;
                case VK_DELETE: commands.push(cpp_conv::commands::CommandType::DeleteSelection); break;
                case VK_RETURN: commands.push(cpp_conv::commands::CommandType::PlaceSelection); break;
                case VK_NEXT: commands.push(cpp_conv::commands::CommandType::MoveFloorDown); break;
                case VK_PRIOR: commands.push(cpp_conv::commands::CommandType::MoveFloorUp); break;
                case VK_UP: commands.push(cpp_conv::commands::CommandType::MoveDown); break;
                case VK_DOWN: commands.push(cpp_conv::commands::CommandType::MoveUp); break;
                case VK_LEFT: commands.push(cpp_conv::commands::CommandType::MoveLeft); break;
                case VK_RIGHT: commands.push(cpp_conv::commands::CommandType::MoveRight); break;
                }
                break;
            case MOUSE_EVENT:
                if (recordBuffer[i].Event.MouseEvent.dwEventFlags != MOUSE_MOVED)
                {
                    switch (recordBuffer[i].Event.MouseEvent.dwEventFlags)
                    {
                    case MOUSE_WHEELED:
                        if (recordBuffer[i].Event.MouseEvent.dwButtonState >> 24)
                        {
                            commands.push(cpp_conv::commands::CommandType::DecrementZoom);
                        }
                        else
                        {
                            commands.push(cpp_conv::commands::CommandType::IncrementZoom);
                        }
                        break;
                    }
                }
                
                break;
            case WINDOW_BUFFER_SIZE_EVENT:
                /*static COORD previousBufferSize = {};
                COORD newSize = recordBuffer[i].Event.WindowBufferSizeEvent.dwSize;
                cpp_conv::apphost::setAppDimensions(newSize.X, newSize.Y);*/
                break;
            }
        }
    }
}
