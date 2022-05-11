#pragma once
#include <cassert>
#include <cstdint>
#include <memory>
#include <string_view>

namespace atlas::core
{
    class FileReader
    {
    public:
        FileReader(uint8_t* data, const size_t size)
            : m_BufferBase{data}
            , m_CurrentPosition{data}
            , m_BufferEnd{data + size - 1}
        {
        }

        template<typename T>
        T Read()
        {
            assert(sizeof(T) + m_CurrentPosition - 1 <= m_BufferEnd);
            T* value = reinterpret_cast<T*>(m_CurrentPosition);
            m_CurrentPosition += sizeof(T);
            return *value;
        }

        template<typename T>
        T* ReadRaw()
        {
            assert(sizeof(T) + m_CurrentPosition - 1 <= m_BufferEnd);
            T* value = reinterpret_cast<T*>(m_CurrentPosition);
            m_CurrentPosition += sizeof(T);
            return value;
        }

        std::unique_ptr<uint8_t[]> ReadBuffer(const size_t size)
        {
            assert(size + m_CurrentPosition - 1 <= m_BufferEnd);
            std::unique_ptr<uint8_t[]> buffer{new uint8_t[size]};
            std::memcpy(buffer.get(), m_CurrentPosition, size);
            m_CurrentPosition += size;
            return buffer;
        }

        void ReadBuffer(uint8_t* buffer, const size_t size)
        {
            assert(size + m_CurrentPosition - 1 <= m_BufferEnd);
            std::memcpy(buffer, m_CurrentPosition, size);
            m_CurrentPosition += size;
        }

        [[nodiscard]] std::string_view ReadStringRange(const size_t start, const size_t end) const
        {
            auto charValue = reinterpret_cast<const char*>(m_BufferBase);
            return { charValue + start, end - start + 1 };
        }

        [[nodiscard]] bool IsEndOfFile() const { return m_CurrentPosition > m_BufferEnd; }

        [[nodiscard]] const uint8_t* GetData() const { return m_BufferBase; }

        [[nodiscard]] size_t GetTotalSize() const { return m_BufferEnd - m_BufferBase + 1; }

    private:
        uint8_t* m_BufferBase;
        uint8_t* m_CurrentPosition;
        uint8_t* m_BufferEnd;
    };
}
