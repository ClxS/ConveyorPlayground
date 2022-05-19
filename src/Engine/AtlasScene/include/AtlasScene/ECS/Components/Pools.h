#pragma once

#include <cassert>
#include <cstdint>
#include <vector>

#include "Archetype.h"
#include "AtlasScene/ECS/Entity.h"

namespace atlas::scene
{
    class PoolBase
    {
    public:
        virtual ~PoolBase() = default;

        [[nodiscard]] virtual int32_t Size() const = 0;
        virtual void Remove(int32_t uiIndex) = 0;
        virtual void SwapAndPop(int32_t uiRemovedIndex) = 0;
        virtual void Pop() = 0;

        virtual void ClaimFromOtherPool(PoolBase* pOther, int32_t otherIndex) = 0;
    };

    template <typename TDataType, bool TIsSparse, uint32_t TInitialSize>
    class Pool : public PoolBase
    {
    public:
        Pool()
        {
            m_Data.reserve(TInitialSize);
        }

        ~Pool() override = default;

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

        [[nodiscard]] TDataType MoveOut(int32_t uiIndex)
        {
            assert(uiIndex >= 0 && uiIndex < m_Data.size());
            TDataType value = std::move(m_Data[uiIndex]);
            return value;
        }

        [[nodiscard]] TDataType GetCopy(int32_t uiIndex) const
        {
            assert(uiIndex >= 0 && uiIndex < m_Data.size());
            return m_Data[uiIndex];
        }

        TDataType& Push(TDataType&& data)
        {
            m_Data.push_back(std::forward<TDataType>(data));
            return m_Data.back();
        }

        template <typename...TValues>
        TDataType& Push(TValues&&... data)
        {
            m_Data.emplace_back(std::forward<TValues&&>(data)...);
            return m_Data.back();
        }

        void Pop() override
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

        template <typename... TArgs>
        void PlacementSet(uint32_t uiIndex, TArgs&&... args)
        {
            new(&m_Data[uiIndex]) TDataType(std::forward<TArgs&&>(args)...);
        }

        void SwapAndPop(int32_t uiRemovedIndex) override
        {
            m_Data[uiRemovedIndex] = std::move(m_Data[m_Data.size() - 1]);
            m_Data.pop_back();
        }

        void ClaimFromOtherPool(PoolBase* pOther, int32_t otherIndex) override
        {
            auto* pTypedOther = static_cast<Pool<TDataType, TIsSparse, TInitialSize>*>(pOther);
            Push(pTypedOther->MoveOut(otherIndex));

            if (pTypedOther->Size() > 1 || otherIndex != pTypedOther->Size() - 1)
            {
                pTypedOther->SwapAndPop(otherIndex);
            }
            else
            {
                pTypedOther->Pop();
            }
        }

        [[nodiscard]] int32_t Size() const
        {
            return static_cast<int32_t>(m_Data.size());
        }

        [[nodiscard]] const std::vector<TDataType>& GetData() const { return m_Data; }

    private:
        std::vector<TDataType> m_Data;
    };

    struct EntityIndex
    {
        int32_t m_EntityIndex = -1;
        ArchetypeIndex m_ArchetypeIndex = ArchetypeIndex::Empty();
    };

    template <typename TComponentType>
    using ComponentPool = Pool<TComponentType, false, 1 << 8>;

    using EntityPool = Pool<EntityId, false, 1 << 8>;
    using EntityIndicesPool = Pool<EntityIndex, true, 1 << 8>;
}
