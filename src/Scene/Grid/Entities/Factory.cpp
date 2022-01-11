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
#include "Profiler.h"
#include "GeneralItemContainer.h"

cpp_conv::Factory::Factory(int x, int y, Direction direction, FactoryId factoryId, uint32_t uiMaxStackSize)
    : Entity(x, y, EntityKind::Producer)
    , m_hFactoryId(factoryId)
    , m_hActiveRecipeId(RecipeIds::None)
    , m_direction(direction)
    , m_uiRemainingCurrentProductionEffort(0)
    , m_bIsRecipeDemandSatisfied(false)
    , m_inputItems(8, uiMaxStackSize, true)
    , m_outputItems(8, uiMaxStackSize, true)
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
        if (!m_outputItems.CouldInsert(pItem.m_idItem, pItem.m_uiCount))
        {
            return false;
        }
    }

    for (const auto& pItem : vOutputItems)
    {
        m_outputItems.TryInsert(pItem.m_idItem, pItem.m_uiCount);
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

bool cpp_conv::Factory::TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem)
{
    return m_outputItems.TryTake(bSingle, outItem);
}

std::string cpp_conv::Factory::GetDescription() const
{
    const cpp_conv::resources::AssetPtr<cpp_conv::RecipeDefinition> pRecipe = cpp_conv::resources::getRecipeDefinition(m_hActiveRecipeId);
    if (!pRecipe)
    {
        return "Producing Unknown Recipe";
    }

    return std::format("Producing {}", pRecipe->GetName());
}

bool cpp_conv::Factory::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, ItemId pItem, int iSourceChannel)
{
    if (pSourceEntity.m_eEntityKind != EntityKind::Inserter)
    {
        // We only allow inserters
        return false;
    }
    const cpp_conv::resources::AssetPtr<cpp_conv::RecipeDefinition> pRecipe = cpp_conv::resources::getRecipeDefinition(m_hActiveRecipeId);
    if (!pRecipe)
    {
        return false;
    }

    bool bIsRequirement = false;
    for (auto& rItem : pRecipe->GetInputItems())
    {
        if (rItem.m_idItem == pItem)
        {
            bIsRequirement = true;
            break;
        }
    }

    if (!bIsRequirement)
    {
        return false;
    }

    return m_inputItems.TryInsert(pItem);
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
    if (!pFactory->HasOwnOutputPipe() || m_outputItems.IsEmpty())
    {
        return;
    }

    cpp_conv::Entity* pEntity = kContext.m_rMap.GetEntity(cpp_conv::grid::GetForwardPosition(*this));
    if (!pEntity || !pEntity->SupportsInsertion())
    {
        return;
    }

    auto& vContainerItems = m_outputItems.GetItems();
    auto itItems = vContainerItems.begin();
    while(itItems != vContainerItems.end())
    {
        for (uint32_t i = 0; i < itItems->m_pCount; ++i)
        {
            if (!pEntity->TryInsert(kContext, *this, itItems->m_pItem, (m_uiTick + i) % cpp_conv::c_conveyorChannels))
            {
                break;
            }

            itItems->m_pCount--;
        }

        if (itItems->m_pCount == 0)
        {
            itItems = vContainerItems.erase(itItems);
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
        for (auto& rStorageItem : m_inputItems.GetItems())
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
        m_inputItems.TryTake(pItem.m_idItem, pItem.m_uiCount);
    }

    return true;
}
