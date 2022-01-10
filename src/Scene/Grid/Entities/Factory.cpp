#include "Factory.h"
#include "Conveyor.h"
#include "SceneContext.h"

#include "Renderer.h"
#include "RenderContext.h"

#include <array>
#include <random>
#include "ConsoleWriteUtility.h"
#include "ResourceManager.h"
#include "ResourceRegistry.h"
#include "TileAsset.h"
#include "DataId.h"
#include "AssetPtr.h"
#include "FactoryRegistry.h"
#include "FactoryDefinition.h"
#include "RecipeRegistry.h"
#include "RecipeDefinition.h"
#include <vector>

cpp_conv::Factory::Factory(int x, int y, Direction direction, FactoryId factoryId, uint32_t uiMaxStackSize)
    : Entity(x, y, EntityKind::Producer)
    , m_hFactoryId(factoryId)
    , m_hActiveRecipeId(RecipeIds::None)
    , m_direction(direction)
    , m_uiRemainingCurrentProductionEffort(0)
    , m_bIsRecipeDemandSatisfied(false)
    , m_uiMaxStackSize(uiMaxStackSize)
    , m_uiTick(0)
{
    const cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition> pFactory = cpp_conv::resources::getFactoryDefinition(factoryId);
    if (!pFactory)
    {
        return;
    }
}

bool cpp_conv::Factory::IsReadyToProduce() const
{
    return m_uiRemainingCurrentProductionEffort == 0;
}

bool cpp_conv::Factory::ProduceItems()
{
    const cpp_conv::resources::AssetPtr<cpp_conv::RecipeDefinition> pRecipe = cpp_conv::resources::getRecipeDefinition(m_hActiveRecipeId);
    if (!pRecipe)
    {
        return false;
    }

    const auto& vOutputItems = pRecipe->GetOutputItems();
    if (vOutputItems.empty())
    {
        return true;
    }

    for (const auto& pItem : vOutputItems)
    {
        bool bIsMet = true;
        for (auto& rStorageItem : m_vProducedItems)
        {
            if (rStorageItem.m_pItem == pItem.m_idItem && (rStorageItem.m_pCount + pItem.m_uiCount) > m_uiMaxStackSize)
            {
                bIsMet = false;
                break;
            }
        }

        if (!bIsMet)
        {
            return false;
        }
    }

    for (const auto& pItem : vOutputItems)
    {
        bool bIsMet = false;
        for (auto& rStorageItem : m_vProducedItems)
        {
            if (rStorageItem.m_pItem == pItem.m_idItem)
            {
                rStorageItem.m_pCount += pItem.m_uiCount;
                bIsMet = true;
                break;
            }
        }

        if (!bIsMet)
        {
            m_vProducedItems.emplace_back(pItem.m_idItem, pItem.m_uiCount);
        }
    }

    return true;
}

void cpp_conv::Factory::Tick(const SceneContext& kContext)
{
    PROFILE_FUNC();
    const cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition> pFactory = cpp_conv::resources::getFactoryDefinition(m_hFactoryId);
    if (!pFactory)
    {
        return;
    }

    m_uiTick++;
    RunProductionCycle(pFactory.get());
    RunOutputCycle(kContext, pFactory.get());
}

void cpp_conv::Factory::Draw(RenderContext& kRenderContext) const
{
    const auto pFactory = cpp_conv::resources::getFactoryDefinition(m_hFactoryId);
    if (!pFactory)
    {
        return;
    }

    auto pTile = pFactory->GetTile();
    if (!pTile)
    {
        return;
    }

    cpp_conv::renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            m_position.m_x * cpp_conv::renderer::c_gridScale,
            m_position.m_y * cpp_conv::renderer::c_gridScale,
            cpp_conv::rotationFromDirection(m_direction)
        },
        { 0xFFFFFF00 });
}

void cpp_conv::Factory::RunProductionCycle(const cpp_conv::FactoryDefinition* pFactory)
{
    if (m_hActiveRecipeId != pFactory->GetProducedRecipe())
    {
        m_hActiveRecipeId = pFactory->GetProducedRecipe();
        m_bIsRecipeDemandSatisfied = false;
    }

    if (!m_bIsRecipeDemandSatisfied)
    {
        if (!TrySatisfyRecipeInput(pFactory, m_uiRemainingCurrentProductionEffort))
        {
            return;
        }

        m_bIsRecipeDemandSatisfied = true;
    }

    if (!IsReadyToProduce())
    {
        m_uiRemainingCurrentProductionEffort -= pFactory->GetProductionRate();
        if (!IsReadyToProduce())
        {
            return;
        }
    }

    if (ProduceItems())
    {
        m_bIsRecipeDemandSatisfied = false;
    }
}

void cpp_conv::Factory::RunOutputCycle(const SceneContext& kContext, const cpp_conv::FactoryDefinition* pFactory)
{
    if (!pFactory->HasOwnOutputPipe() || m_vProducedItems.empty())
    {
        return;
    }

    cpp_conv::Entity* pEntity = cpp_conv::grid::SafeGetEntity(kContext.m_grid, cpp_conv::grid::GetForwardPosition(*this));
    if (!pEntity || !pEntity->SupportsInsertion())
    {
        return;
    }

    auto itItems = m_vProducedItems.begin();
    while (itItems != m_vProducedItems.end())
    {
        for (int i = 0; i < itItems->m_pCount; ++i)
        {
            if (!pEntity->TryInsert(kContext, *this, itItems->m_pItem, (m_uiTick + i) % cpp_conv::c_conveyorChannels))
            {
                break;
            }

            itItems->m_pCount--;
        }

        if (itItems->m_pCount == 0)
        {
            itItems = m_vProducedItems.erase(itItems);
        }
        else
        {
            break;
        }
    }
}

bool cpp_conv::Factory::TrySatisfyRecipeInput(const cpp_conv::FactoryDefinition* pFactory, uint64_t& uiOutEffort)
{
    const cpp_conv::resources::AssetPtr<cpp_conv::RecipeDefinition> pRecipe = cpp_conv::resources::getRecipeDefinition(m_hActiveRecipeId);
    if (!pRecipe)
    {
        return false;
    }

    uiOutEffort = pRecipe->GetEffort();
    auto& vInputItems = pRecipe->GetInputItems();
    if (vInputItems.empty())
    {
        return true;
    }

    for (const auto& pItem : vInputItems)
    {
        bool bIsMet = false;
        for (auto& rStorageItem : m_vRecipeItemStorage)
        {
            if (rStorageItem.m_pItem == pItem.m_idItem && rStorageItem.m_pCount >= pItem.m_uiCount)
            {
                bIsMet = true;
                break;
            }
        }

        if (!bIsMet)
        {
            return false;
        }
    }

    for (const auto& pItem : vInputItems)
    {
        for (auto& rStorageItem : m_vRecipeItemStorage)
        {
            if (rStorageItem.m_pItem == pItem.m_idItem && rStorageItem.m_pCount == pItem.m_uiCount)
            {
                rStorageItem.m_pCount -= pItem.m_uiCount;
            }
        }
    }

    return true;
}
