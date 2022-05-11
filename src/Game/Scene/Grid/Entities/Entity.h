#pragma once

#include <cstdint>
#include "DataId.h"
#include "Enums.h"

namespace cpp_conv
{
    class WorldMap;
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
            Eigen::Vector2f m_OriginPosition = {};

        public:
            explicit InsertInfo(const ItemId item)
                : m_Item(item)
            {
            }

            explicit InsertInfo(const ItemId item, const uint8_t sourceChannel)
                : m_Item(item)
                  , m_SourceChannel{sourceChannel}
                  , m_bHasChannelInfo(true)
            {
            }

            explicit InsertInfo(const ItemId item, const uint8_t sourceChannel, Eigen::Vector2f originPosition)
                : m_Item(item)
                  , m_SourceChannel{sourceChannel}
                  , m_bHasChannelInfo(true)
                  , m_bHasPosition{true}
                  , m_OriginPosition{originPosition}
            {
            }

            [[nodiscard]] ItemId GetItem() const { return m_Item; }
            [[nodiscard]] uint8_t GetSourceChannel() const { return m_SourceChannel; }
            [[nodiscard]] bool HasChannelInformation() const { return m_bHasChannelInfo; }
            [[nodiscard]] bool HasOriginPosition() const { return m_bHasPosition; }
            [[nodiscard]] Eigen::Vector2f GetOriginPosition() const { return m_OriginPosition; }
        };

        Entity(const Eigen::Vector3i position, const Eigen::Vector3i size, const EntityKind eEntityKind,
               const Direction direction = Direction::Right)
            : m_position(position)
              , m_size(size)
              , m_eEntityKind(eEntityKind)
              , m_Direction{direction}
        {
        }

        virtual ~Entity() = default;

        Eigen::Vector3i m_position;
        Eigen::Vector3i m_size;
        EntityKind m_eEntityKind;
        Direction m_Direction;
    };
}
