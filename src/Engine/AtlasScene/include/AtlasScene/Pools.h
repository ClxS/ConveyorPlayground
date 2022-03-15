#pragma once

#include <cassert>
#include <cstdint>
#include <vector>

#include "AtlasScene/Entity.h"

namespace atlas::scene
{
    class PoolBase
    {
    public:
        virtual ~PoolBase() = default;

        virtual void PushEmpty() = 0;
        virtual void Remove(int32_t uiIndex) = 0;
        virtual void SwapAndPop(int32_t uiRemovedIndex) = 0;
        virtual void Pop() = 0;
    };

    template<typename TDataType, bool TIsSparse, uint32_t TInitialSize>
    class Pool : public PoolBase
    {
    public:
        Pool()
        {
            m_Data.reserve(TInitialSize);
        }
        virtual ~Pool() = default;

        [[nodiscard]] TDataType& GetReference(int32_t uiIndex)
        {
            assert(uiIndex >= 0 && uiIndex < m_Data.size());
            return m_Data[uiIndex];
        }

        [[nodiscard]] const TDataType& GetReference(int32_t uiIndex) const
        {
            assert(uiIndex >= 0 && uiIndex < m_Data.size());
            return m_Data[uiIndex];
        }

        [[nodiscard]] TDataType GetCopy(int32_t uiIndex) const
        {
            assert(uiIndex >= 0 && uiIndex < m_Data.size());
            return m_Data[uiIndex];
        }

        void PushEmpty() override
        {
            m_Data.push_back({});
        }

        void Push(TDataType data)
        {
            m_Data.push_back(data);
        }

        void Pop()
        {
            m_Data.pop_back();
        }

        void Remove(int32_t uiIndex) override
        {
            assert(uiIndex >= 0 && uiIndex < m_Data.size());
            if constexpr (TIsSparse)
            {
                m_Data[uiIndex] = {};
            }
            else
            {
                m_Data.erase(m_Data.begin() + uiIndex);
            }
        }

        void Set(int32_t uiIndex, const TDataType& value)
        {
            m_Data[uiIndex] = value;
        }

        void Set(int32_t uiIndex, TDataType&& value)
        {
            m_Data[uiIndex] = value;
        }

        template<typename... TArgs>
        void PlacementSet(uint32_t uiIndex, TArgs&&... args)
        {
            new (&m_Data[uiIndex]) TDataType( std::forward<TArgs&&>(args)... );
        }

        void SwapAndPop(int32_t uiRemovedIndex)
        {
            m_Data[uiRemovedIndex] = m_Data[m_Data.size() - 1];
            m_Data.pop_back();
        }

        [[nodiscard]] int32_t Size() const
        {
            return static_cast<int32_t>(m_Data.size());
        }

    private:
        std::vector<TDataType> m_Data;
    };

    struct EntityInfo
    {
        EntityId m_EntityId = EntityId::Invalid();
        uint64_t m_PossessedComponentMask = 0;
    };

    template<typename TComponentType>
    using ComponentPool = Pool<TComponentType, false, 1 << 16>;

    using EntityPool = Pool<EntityInfo, false, 1 << 16>;
    using EntityIndicesPool = Pool<int32_t, true, 1 << 16>;
}
