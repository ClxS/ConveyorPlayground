#include "ConsoleScreenBuffer.h"
#include "WriteSurface.h"
#include "SwapChain.h"

cpp_conv::renderer::ConsoleScreenBuffer::ConsoleScreenBuffer(ConsoleWriteSurface& rWriteSurface)
    : m_rWriteSurface(rWriteSurface)
    , m_hBufferHandle(0)
{
}

cpp_conv::renderer::ConsoleScreenBuffer::~ConsoleScreenBuffer()
{
    Shutdown();
}

void cpp_conv::renderer::ConsoleScreenBuffer::Initialize(RenderContext& kRenderContext, ConsoleScreenBufferInitArgs& rArgs)
{
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof cfi;
    cfi.nFont = 0;
    cfi.dwFontSize.X = (SHORT)(18 * kRenderContext.m_fZoom);
    cfi.dwFontSize.Y = (SHORT)(18 * kRenderContext.m_fZoom);
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_HEAVY;

    wcscpy_s<32>(cfi.FaceName, L"Lucida Console");

    m_initArgs = rArgs;
    const COORD coordBufSize = { (SHORT)m_rWriteSurface.GetWidth(), (SHORT)m_rWriteSurface.GetHeight() };
    SMALL_RECT srctWriteRect;
    srctWriteRect.Left = srctWriteRect.Top = 0;
    srctWriteRect.Right = m_rWriteSurface.GetWidth() - 1;
    srctWriteRect.Bottom = m_rWriteSurface.GetHeight() - 1;

    m_hBufferHandle = CreateConsoleScreenBuffer(
        GENERIC_WRITE,
        0,
        NULL,                    // default security attributes 
        CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE 
        NULL);                   // reserved; must be NULL 

    if (cfi.cbSize != 0)
    {
        SetCurrentConsoleFontEx(m_hBufferHandle, FALSE, &cfi);
    }

    SetConsoleScreenBufferSize(m_hBufferHandle, coordBufSize);
}

void cpp_conv::renderer::ConsoleScreenBuffer::Present()
{
    const COORD coordBufSize = { (SHORT)m_rWriteSurface.GetWidth(), (SHORT)m_rWriteSurface.GetHeight() };
    const COORD coordBufCoord = { 0, 0 };
    SMALL_RECT srctWriteRect;
    srctWriteRect.Left = srctWriteRect.Top = 0;
    srctWriteRect.Right = m_rWriteSurface.GetWidth() - 1;
    srctWriteRect.Bottom = m_rWriteSurface.GetHeight() - 1;

    BOOL fSuccess = WriteConsoleOutput(
        m_hBufferHandle,
        m_rWriteSurface.GetData().data(),
        coordBufSize,     // col-row size of chiBuffer 
        coordBufCoord,    // top left src cell in chiBuffer 
        &srctWriteRect);

    if (!SetConsoleActiveScreenBuffer(m_hBufferHandle))
    {
        printf("SetConsoleActiveConsoleScreenBuffer failed - (%d)\n", GetLastError());
    }

    if (m_rWriteSurface.IsClearOnPresent())
    {
        m_rWriteSurface.Clear();
    }

    ShowScrollBar(GetConsoleWindow(), SB_BOTH, FALSE);
}

void cpp_conv::renderer::ConsoleScreenBuffer::Shutdown()
{
    if (m_hBufferHandle == 0)
    {
        return;
    }

    CloseHandle(m_hBufferHandle);
    m_hBufferHandle = 0;
}

void cpp_conv::renderer::ConsoleScreenBuffer::RecreateBuffer(RenderContext& kRenderContext)
{
    Shutdown();
    Initialize(kRenderContext, m_initArgs);
}
