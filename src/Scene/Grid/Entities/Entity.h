#pragma once

#include <cstdint>
#include "Enums.h"
#include "Vector3.h"
#include "DataId.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    class Entity
    {
    public:
        class InsertInfo
        {
            ItemId m_Item = {};
            uint8_t m_SourceChannel = {};
            bool m_bHasChannelInfo = {};
            bool m_bHasPosition = {};
            Vector2F m_OriginPosition = {};

        public:
            explicit InsertInfo(const ItemId item)
                : m_Item(item) {}

            explicit InsertInfo(const ItemId item, const uint8_t sourceChannel)
                : m_Item(item)
                , m_SourceChannel{sourceChannel}
                , m_bHasChannelInfo(true)
            {}

            explicit InsertInfo(const ItemId item, const uint8_t sourceChannel, Vector2F originPosition)
                : m_Item(item)
                , m_SourceChannel{sourceChannel}
                , m_bHasChannelInfo(true)
                , m_bHasPosition{true}
                , m_OriginPosition{originPosition}
            {}

            [[nodiscard]] ItemId GetItem() const { return m_Item; }
            [[nodiscard]] uint8_t GetSourceChannel() const { return m_SourceChannel; }
            [[nodiscard]] bool HasChannelInformation() const { return m_bHasChannelInfo; }
            [[nodiscard]] bool HasOriginPosition() const { return m_bHasPosition; }
            [[nodiscard]] Vector2F GetOriginPosition() const { return m_OriginPosition; }
        };

        Entity(const Vector3 position, const Vector3 size, const EntityKind eEntityKind)
            : m_position(position)
            , m_size(size)
            , m_eEntityKind(eEntityKind)
        {
        }
        virtual ~Entity() = default;

        virtual void Tick(const SceneContext& kContext) = 0;
        virtual void Draw(RenderContext& kContext) const = 0;

        virtual bool TryInsert(
            const SceneContext& kContext,
            const Entity& pSourceEntity,
            InsertInfo insertInfo)
        {
            return false;
        }

        virtual bool TryGrab(
            const SceneContext& kContext,
            bool bSingle,
            std::tuple<ItemId, uint32_t>& outItem)
        {
            return false;
        }

        [[nodiscard]] virtual bool SupportsInsertion() const { return false; }
        [[nodiscard]] virtual bool SupportsProvidingItem() const { return false; }
        [[nodiscard]] virtual Direction GetDirection() const { return Direction::Left; }
        [[nodiscard]] virtual const char* GetName() const = 0;
        [[nodiscard]] virtual std::string GetDescription() const = 0;
        [[nodiscard]] virtual uint32_t GetDrawPassCount() const { return 1; }

        Vector3 m_position;
        Vector3 m_size;
        const EntityKind m_eEntityKind;
    };
}
