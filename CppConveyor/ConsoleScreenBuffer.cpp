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

void cpp_conv::renderer::ConsoleScreenBuffer::Initialize(ConsoleScreenBufferInitArgs& rArgs)
{
	m_initArgs = rArgs;
	COORD coordBufSize = { (SHORT)m_rWriteSurface.GetWidth(), (SHORT)m_rWriteSurface.GetHeight() };
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

	if (rArgs.m_surfaceFont.cbSize != 0)
	{
		SetCurrentConsoleFontEx(m_hBufferHandle, FALSE, &rArgs.m_surfaceFont);
	}

	SetConsoleScreenBufferSize(m_hBufferHandle, coordBufSize);
	SetConsoleWindowInfo(m_hBufferHandle, TRUE, &srctWriteRect);
}

void cpp_conv::renderer::ConsoleScreenBuffer::Present()
{
	COORD coordBufSize = { (SHORT)m_rWriteSurface.GetWidth(), (SHORT)m_rWriteSurface.GetHeight() };
	COORD coordBufCoord = { 0, 0 };
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

	SetConsoleCursorPosition(m_hBufferHandle, coordBufCoord);
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

void cpp_conv::renderer::ConsoleScreenBuffer::RecreateBuffer()
{
	Shutdown();
	Initialize(m_initArgs);
}
