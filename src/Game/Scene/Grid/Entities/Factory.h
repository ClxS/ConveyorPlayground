#pragma once

#include "DataId.h"
#include "Entity.h"
#include "GeneralItemContainer.h"
#include "Renderer.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;
    class FactoryDefinition;

    class Factory : public Entity
    {
    public:
        Factory(
            Eigen::Vector3i position,
            Direction direction,
            FactoryId factoryId)
        : Entity(position, {1, 1, 1}, EntityKind::Producer, direction)
          , m_hFactoryId(factoryId)
          , m_hActiveRecipeId(RecipeIds::None)
          , m_uiRemainingCurrentProductionEffort(0)
          , m_uiTick(0)
          , m_bIsRecipeDemandSatisfied(false)
        {
        }

        [[nodiscard]] const char* GetName() const { return "Factory"; }
        [[nodiscard]] std::string GetDescription() const;

        [[nodiscard]] bool HasOutputPipe() const { return m_bHasOutputPipe; }
        [[nodiscard]] Eigen::Vector3i GetOutputPipe() const { return m_OutputPipe; }

        [[nodiscard]] FactoryId GetDefinitionId() const { return m_hFactoryId; }

    private:
        FactoryId m_hFactoryId;
        RecipeId m_hActiveRecipeId;

        uint64_t m_uiRemainingCurrentProductionEffort;
        uint64_t m_uiTick;

        Eigen::Vector3i m_OutputPipe;
        bool m_bHasOutputPipe;

        bool m_bIsRecipeDemandSatisfied;
    };
}
