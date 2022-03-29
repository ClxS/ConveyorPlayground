#pragma once
#include <optional>

#include "FactoryDefinition.h"
#include "GeneralItemContainer.h"
#include "Eigen/src/Core/Matrix.h"

namespace cpp_conv::components
{
    struct FactoryComponent
    {
        struct RecipeItem
        {
            ItemId m_Item;
            uint32_t m_Count;
        };

        struct Recipe
        {
            uint32_t m_Effort{};
            std::vector<RecipeItem> m_InputItems;
            std::vector<RecipeItem> m_OutputItems;
        };

        FactoryComponent()
            : m_InputItems{256, 64, true}
              , m_OutputItems{256, 64, true}
              , m_ProductionRate{0}
              , m_Tick{0}
              , m_RemainingCurrentProductionEffort{0}
              , m_bIsDemandSatisfied{false}
        {
        }

        GeneralItemContainer m_InputItems;
        GeneralItemContainer m_OutputItems;

        Eigen::Vector3i m_Size;
        std::optional<Recipe> m_Recipe;
        std::optional<Eigen::Vector3i> m_OutputPipe;
        uint32_t m_ProductionRate;
        uint32_t m_Tick;
        uint32_t m_RemainingCurrentProductionEffort;


        bool m_bIsDemandSatisfied;
    };
}
