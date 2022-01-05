#pragma once

#include <cstdint>
#include <vector>
#include <Windows.h>

namespace cpp_conv::renderer
{
	class WriteSurface
	{
	public:
		WriteSurface(uint16_t uiWidth, uint16_t uiHeight, bool bClearOnPresent);

		void Initialize();

		void Clear();

		std::vector<CHAR_INFO>& GetData() { return m_chiBuffer; }
		const std::vector<CHAR_INFO>& GetData() const { return m_chiBuffer; }

		uint16_t GetWidth() const { return m_uiWidth; }
		uint16_t GetHeight() const { return m_uiHeight; }
		bool IsClearOnPresent() const { return m_bClearOnPresent; }

	private:
		uint16_t m_uiWidth;
		uint16_t m_uiHeight;
		bool m_bClearOnPresent;

		std::vector<CHAR_INFO> m_chiBuffer;
	};
}