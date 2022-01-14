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
        Entity(Vector3 position, Vector3 size, EntityKind eEntityKind)
            : m_position(position)
            , m_size(size)
            , m_eEntityKind(eEntityKind)
        {
        }
        virtual ~Entity() = default;

        virtual void Tick(const SceneContext& kContext) = 0;
        virtual void Draw(RenderContext& kContext) const = 0;
        virtual bool SupportsInsertion() const { return false; }
        virtual bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, ItemId pItem, int iSourceChannel) { return false; }

        virtual bool SupportsProvidingItem() const { return false; }
        virtual bool TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem) { return false; }

        virtual Direction GetDirection() const { return Direction::Left; }

        virtual const char* GetName() const = 0;
        virtual std::string GetDescription() const = 0;

        virtual uint32_t GetDrawPassCount() const { return 1; }

        Vector3 m_position;
        Vector3 m_size;
        const EntityKind m_eEntityKind;
    };
}
