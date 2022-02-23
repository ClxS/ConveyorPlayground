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
        explicit FixedCircularBuffer(uint32_t uiCapacity)
			: m_vData(uiCapacity)
			, m_uiConsumePivot(0)
			, m_uiAppendPivot(0)
			, m_uiSize(0)
			, m_uiCapacity(uiCapacity)
		{
		}

        constexpr void Push(const T& item)
		{
			assert(m_uiSize < m_vData.size());
			m_vData[m_uiAppendPivot] = item;
            IncrementAppendPosition();
		}

        constexpr T Pop()
		{
			assert(m_uiSize > 0);
			T value = m_vData[m_uiConsumePivot];
			m_uiConsumePivot = (m_uiConsumePivot + 1) % m_uiCapacity;
			--m_uiSize;
			return value;
		}

        T Remove(uint32_t index = 0);

	    [[nodiscard]] uint32_t GetSize() const { return m_uiSize; }

        constexpr void Insert(const uint64_t uiIndex, T item)
        {
            assert(uiIndex <= m_uiSize);
            assert(m_uiSize < m_uiCapacity);
            if (uiIndex == m_uiSize)
            {
                Push(item);
                return;
            }

            uint32_t iStartIndex = (m_uiConsumePivot + uiIndex) % m_uiCapacity;
            const uint32_t iPushForwardCount = m_uiSize;

            if ((iStartIndex + iPushForwardCount) >= m_uiCapacity)
            {
                const uint32_t prewrapMoveSize = m_uiCapacity - iStartIndex;
                const uint32_t postWrapMoveSize = m_uiAppendPivot;

                std::memcpy(&m_vData[1], &m_vData[0], postWrapMoveSize * sizeof(T));
                m_vData[0] = m_vData[m_uiCapacity - 1];
                std::memcpy(&m_vData[iStartIndex + 1], &m_vData[iStartIndex], prewrapMoveSize * sizeof(T));
                m_vData[iStartIndex] = item;
            }
            else
            {
                std::memcpy(&m_vData[iStartIndex + 1], &m_vData[iStartIndex], iPushForwardCount * sizeof(T));
                m_vData[iStartIndex] = item;
            }

            IncrementAppendPosition();
        }

        [[nodiscard]] constexpr const T& Peek(const int index = 0) const
		{
			assert(m_uiSize > 0);
			return m_vData[(m_uiConsumePivot + index) % m_uiCapacity];
		}

	private:
        void IncrementAppendPosition()
        {
            m_uiAppendPivot = (m_uiAppendPivot + 1) % m_uiCapacity;
            ++m_uiSize;
        }

		std::vector<T> m_vData;
		uint32_t m_uiConsumePivot;
		uint32_t m_uiAppendPivot;
		uint32_t m_uiSize;
		uint32_t m_uiCapacity;
	};

    template<typename T>
    T FixedCircularBuffer<T>::Remove(uint32_t index /*= 0*/)
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

        const uint32_t toMove = m_uiSize - index;
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
            const uint32_t prePivotMove = toMove - m_uiAppendPivot - 1;
            const uint32_t postPivotMove = m_uiAppendPivot;

            std::memmove(&m_vData[uiTakeIndex], &m_vData[uiTakeIndex] + 1, sizeof(T) * prePivotMove);
            m_vData[m_uiCapacity - 1] = m_vData[0];
            std::memmove(&m_vData[0], &m_vData[1], sizeof(T) * postPivotMove);
        }

        return value;
    }

}
