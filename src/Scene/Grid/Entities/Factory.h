#pragma once

#include "Renderer.h"
#include "Entity.h"
#include "DataId.h"
#include "GeneralItemContainer.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;
    class FactoryDefinition;

    class Factory : public Entity
    {
    public:
        Factory(
            Vector3 position,
            Direction direction,
            FactoryId factoryId,
            uint32_t uiMaxStackSize = 99);

        bool IsReadyToProduce() const;

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;

        bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, InsertInfo insertInfo) override;

        bool SupportsProvidingItem() const override { return true; }
        bool TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem) override;

        Direction GetDirection() const override { return m_direction; }

        const char* GetName() const override { return "Factory"; }
        std::string GetDescription() const override;

        bool HasOutputPipe() const { return m_bHasOutputPipe; }
        Vector3 GetOutputPipe() const { return m_OutputPipe; }

    private:
        bool ProduceItems();
        void RunProductionCycle(const cpp_conv::FactoryDefinition* pFactory);
        bool TrySatisfyRecipeInput(const cpp_conv::FactoryDefinition* pFactory, uint64_t& uiOutEffort);
        void RunOutputCycle(const SceneContext& kContext, const cpp_conv::FactoryDefinition* pFactory);

        GeneralItemContainer m_inputItems;
        GeneralItemContainer m_outputItems;

        FactoryId m_hFactoryId;
        RecipeId m_hActiveRecipeId;
        Direction m_direction;

        uint64_t m_uiRemainingCurrentProductionEffort;
        uint64_t m_uiTick;

        Vector3 m_OutputPipe;
        bool m_bHasOutputPipe;

        bool m_bIsRecipeDemandSatisfied;
    };
}
