#pragma once

#include "Renderer.h"
#include "Entity.h"
#include "DataId.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;
    class FactoryDefinition;

    class Factory : public Entity
    { 
    public:
        Factory(
            int x,
            int y,
            Direction direction,
            FactoryId factoryId,
            uint32_t uiMaxStackSize = 99);

        bool IsReadyToProduce() const;

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;

        Direction GetDirection() const override { return m_direction; }
    private:
        bool ProduceItems();
        void RunProductionCycle(const cpp_conv::FactoryDefinition* pFactory);
        bool TrySatisfyRecipeInput(const cpp_conv::FactoryDefinition* pFactory, uint64_t& uiOutEffort);
        void RunOutputCycle(const SceneContext& kContext, const cpp_conv::FactoryDefinition* pFactory);

        struct ItemEntry
        {
            ItemId m_pItem;
            uint32_t m_pCount;
        };

        std::vector<ItemEntry> m_vRecipeItemStorage;
        std::vector<ItemEntry> m_vProducedItems;
        uint32_t m_uiMaxStackSize;

        FactoryId m_hFactoryId;
        RecipeId m_hActiveRecipeId;
        Direction m_direction;

        uint64_t m_uiRemainingCurrentProductionEffort;
        uint64_t m_uiTick;

        bool m_bIsRecipeDemandSatisfied;
    };
}
