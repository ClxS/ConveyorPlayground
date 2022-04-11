#pragma once
#include <cassert>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace asset_builder::utility::file_utility
{
    class StructuredFileWriter
    {
        enum class FixupType
        {
            Single,
            DoubleSided
        };

        struct Fixup
        {
            FixupType m_Type;
            uintptr_t m_Base;
        };

    public:
        explicit StructuredFileWriter(const int32_t baseSize = 32)
        {
            m_Buffer.reserve(baseSize);
        }

        void AddSingleSidedFixup(const std::string& key)
        {
            EnsureSizeIncrease(sizeof(int32_t));
            m_FixupPointers[key] = { FixupType::Single, m_Buffer.size() - sizeof(int32_t) };
            memset(&(m_Buffer[m_Buffer.size() - sizeof(int32_t)]), 0, sizeof(int32_t));
        }

        void AddDoubleSizedFixup(const std::string& key)
        {
            EnsureSizeIncrease(sizeof(int32_t) * 2);
            m_FixupPointers[key] = { FixupType::DoubleSided, m_Buffer.size() - sizeof(int32_t) * 2 };
            memset(&(m_Buffer[m_Buffer.size() - sizeof(int32_t) * 2]), 0, sizeof(int32_t) * 2);
        }

        void AddData(const void* data, const size_t size)
        {
            EnsureSizeIncrease(size);
            memcpy(&(m_Buffer[m_Buffer.size() - size]), data, size);
        }

        void AddData(const int32_t value)
        {
            AddData(&value, sizeof(int32_t));
        }

        void AddData(const int64_t value)
        {
            AddData(&value, sizeof(int64_t));
        }

        void AddKeyedData(const std::string& key, const void* data, const size_t size)
        {
            const auto it = m_FixupPointers.find(key);
            assert(it != m_FixupPointers.end());
            const auto fixup = *it;
            m_FixupPointers.erase(it);

            EnsureSizeIncrease(size);
            memcpy(&(m_Buffer[m_Buffer.size() - size]), data, size);

            if (fixup.second.m_Type == FixupType::Single)
            {
                const auto start = static_cast<int32_t>(m_Buffer.size() - size);
                memcpy(&(m_Buffer[fixup.second.m_Base]), &start, sizeof(int32_t));
            }
            else
            {
                const auto start = static_cast<int32_t>(m_Buffer.size() - size);
                const auto end = static_cast<int32_t>(m_Buffer.size() - 1);
                memcpy(&(m_Buffer[fixup.second.m_Base]), &start, sizeof(int32_t));
                memcpy(&(m_Buffer[fixup.second.m_Base + sizeof(int32_t)]), &end, sizeof(int32_t));
            }
        }

        void AddKeyedData(const std::string& key, const std::string& data)
        {
            AddKeyedData(key, data.data(), data.size());
        }

        template<typename T>
        void AddKeyedData(const std::string& key, const std::vector<T>& data)
        {
            AddKeyedData(key, data.data(), data.size() * sizeof(T));
        }

        [[nodiscard]] const std::vector<uint8_t>& GetData() const { return m_Buffer; }
        [[nodiscard]] size_t GetDataSize() const { return m_Buffer.size(); }

        [[nodiscard]] std::unique_ptr<uint8_t[]> GetFinalData() const
        {
            std::unique_ptr<uint8_t[]> ptr{new uint8_t[m_Buffer.size()]};
            memcpy(ptr.get(), m_Buffer.data(), m_Buffer.size());
            return ptr;
        }

    private:
        void EnsureSizeIncrease(const size_t increase)
        {
            while (m_Buffer.size() + increase > m_Buffer.capacity())
            {
                m_Buffer.reserve(m_Buffer.capacity() * 2);
            }

            m_Buffer.resize(m_Buffer.size() + increase);
        }

        std::vector<uint8_t> m_Buffer;
        std::map<std::string, Fixup> m_FixupPointers;
    };
}
