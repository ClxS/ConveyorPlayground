#pragma once

#include "Entity.h"

#include <vector>
#include "GeneralItemContainer.h"

namespace cpp_conv
{
    class Storage : public Entity
    {
    public:
        Storage(Vector3 position, Vector3 size, uint32_t uiMaxCapacity, uint32_t uiMaxStackSize);

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;
        [[nodiscard]] bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, InsertInfo insertInfo) override;

        [[nodiscard]] bool SupportsProvidingItem() const override { return true; }
        bool TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem) override;

        [[nodiscard]] const char* GetName() const override { return "Storage"; }
        [[nodiscard]] std::string GetDescription() const override { return m_itemContainer.GetDescription(); }

    private:
        GeneralItemContainer m_itemContainer;
    };
}
