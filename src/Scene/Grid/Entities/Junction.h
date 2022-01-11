#pragma once

#include "Entity.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    class Junction : public Entity
    {
    public:
        Junction(Vector3 position, Vector3 size);

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;
        bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, ItemId pItem, int iSourceChannel) override;

        const char* GetName() const override { return "Junction"; }
        std::string GetDescription() const override { return ""; }

    private:
        ItemId m_pItem;
        uint64_t m_uiTick;
    };
}
