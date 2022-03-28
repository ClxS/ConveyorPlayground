#pragma once
#include <optional>

#include "FactoryDefinition.h"
#include "RecipeDefinition.h"
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

        Eigen::Vector3i m_Size;
        std::optional<Recipe> m_Recipe;
        std::optional<Eigen::Vector3i> m_OutputPipe;
        uint32_t m_ProductionRate;
    };
}
