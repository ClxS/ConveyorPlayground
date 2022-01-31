#pragma once
#include <vector>
#include <cstring>
#include <cassert>

namespace cpp_conv
{
	template<typename T>
	class FixedCircularBuffer
	{
	public:
		FixedCircularBuffer(uint32_t uiCapacity)
			: m_vData(uiCapacity)
			, m_uiConsumePivot(0)
			, m_uiAppendPivot(0)
			, m_uiSize(0)
			, m_uiCapacity(uiCapacity)
		{
		}

		void Push(const T& item)
		{
			assert(m_uiSize < m_vData.size());
			m_vData[m_uiAppendPivot] = item;
			m_uiAppendPivot = (m_uiAppendPivot + 1) % m_uiCapacity;
			++m_uiSize;
		}

		T Pop()
		{
			assert(m_uiSize > 0);
			T value = m_vData[m_uiConsumePivot];
			m_uiConsumePivot = (m_uiConsumePivot + 1) % m_uiCapacity;
			--m_uiSize;
			return value;
		}

		T Remove(int index = 0)
		{
            assert(m_uiSize > 0);
            assert(index < m_uiSize);

			uint32_t uiTakeIndex = (m_uiConsumePivot + index) % m_uiCapacity;
			T value = m_vData[uiTakeIndex];
			if (m_uiAppendPivot == 0)
			{
				m_uiAppendPivot = m_uiCapacity - 1;
			}
			else
			{
				--m_uiAppendPivot;
			}

            --m_uiSize;

            uint32_t toMove = m_uiSize - index;
            if (m_uiAppendPivot == m_uiConsumePivot || toMove == 0)
            {
                return value;
            }

            if (uiTakeIndex + toMove < m_uiCapacity)
            {
                std::memmove(&m_vData[uiTakeIndex], &m_vData[uiTakeIndex] + 1, sizeof(T) * toMove);
            }
            else
            {
                uint32_t prePivotMove = toMove - m_uiAppendPivot - 1;
                uint32_t postPivotMove = m_uiAppendPivot;

                std::memmove(&m_vData[uiTakeIndex], &m_vData[uiTakeIndex] + 1, sizeof(T) * prePivotMove);
                m_vData[m_uiCapacity - 1] = m_vData[0];
                std::memmove(&m_vData[0], &m_vData[1], sizeof(T) * postPivotMove);
            }
		}

		const T& Peek(int index = 0) const 
		{
			assert(m_uiSize > 0);
			return m_vData[(m_uiConsumePivot + index) % m_uiCapacity];
		}

	private:
		std::vector<T> m_vData;
		uint32_t m_uiConsumePivot;
		uint32_t m_uiAppendPivot;
		uint32_t m_uiSize;
		uint32_t m_uiCapacity;
	};
}
