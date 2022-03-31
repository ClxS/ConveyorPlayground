#include "Factory.h"
#include "Conveyor.h"
#include "SceneContext.h"

#include "RenderContext.h"
#include "Renderer.h"

#include <array>
#include <random>
#include <vector>
#include "AssetPtr.h"
#include "DataId.h"
#include "Direction.h"
#include "FactoryDefinition.h"
#include "FactoryRegistry.h"
#include "GeneralItemContainer.h"
#include "Profiler.h"
#include "RecipeDefinition.h"
#include "RecipeRegistry.h"
#include "ResourceManager.h"
#include "TileAsset.h"

cpp_conv::Factory::Factory(Vector3 position, Direction direction, FactoryId factoryId, uint32_t uiMaxStackSize)
    : Entity(position, {1, 1, 1}, EntityKind::Producer)
      , m_inputItems(8, uiMaxStackSize, true)
      , m_outputItems(8, uiMaxStackSize, true)
      , m_hFactoryId(factoryId)
      , m_hActiveRecipeId(RecipeIds::None)
      , m_direction(direction)
      , m_uiRemainingCurrentProductionEffort(0)
      , m_uiTick(0)
      , m_bIsRecipeDemandSatisfied(false)
{
    const resources::AssetPtr<FactoryDefinition> pFactory = resources::getFactoryDefinition(factoryId);
    if (!pFactory)
    {
        return;
    }

    m_size = pFactory->GetSize();
    m_OutputPipe = pFactory->GetOutputPipe();
    m_bHasOutputPipe = pFactory->HasOwnOutputPipe();
}

bool cpp_conv::Factory::IsReadyToProduce() const
{
    return m_uiRemainingCurrentProductionEffort == 0;
}

bool cpp_conv::Factory::ProduceItems()
{
    const resources::AssetPtr<RecipeDefinition> pRecipe = resources::getRecipeDefinition(m_hActiveRecipeId);
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
    const resources::AssetPtr<FactoryDefinition> pFactory = resources::getFactoryDefinition(m_hFactoryId);
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
    const auto pFactory = resources::getFactoryDefinition(m_hFactoryId);
    if (!pFactory)
    {
        return;
    }

    const auto pTile = pFactory->GetTile();
    if (!pTile)
    {
        return;
    }

    renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            static_cast<float>(m_position.GetX()) * renderer::c_gridScale,
            static_cast<float>(m_position.GetY()) * renderer::c_gridScale,
        },
        {0xFFFFFF00});
}

bool cpp_conv::Factory::TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem)
{
    return m_outputItems.TryTake(bSingle, outItem);
}

std::string cpp_conv::Factory::GetDescription() const
{
    const resources::AssetPtr<RecipeDefinition> pRecipe = resources::getRecipeDefinition(m_hActiveRecipeId);
    if (!pRecipe)
    {
        return "Producing Unknown Recipe";
    }

    return std::format("Producing {}", pRecipe->GetName());
}

bool cpp_conv::Factory::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity,
                                  const InsertInfo insertInfo)
{
    if (pSourceEntity.m_eEntityKind != EntityKind::Inserter)
    {
        // We only allow inserters
        return false;
    }
    const resources::AssetPtr<RecipeDefinition> pRecipe = resources::getRecipeDefinition(m_hActiveRecipeId);
    if (!pRecipe)
    {
        return false;
    }

    bool bIsRequirement = false;
    for (const auto& [item, count] : pRecipe->GetInputItems())
    {
        if (item == insertInfo.GetItem())
        {
            bIsRequirement = true;
            break;
        }
    }

    if (!bIsRequirement)
    {
        return false;
    }

    return m_inputItems.TryInsert(insertInfo.GetItem());
}

void cpp_conv::Factory::RunProductionCycle(const FactoryDefinition* pFactory)
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

void cpp_conv::Factory::RunOutputCycle(const SceneContext& kContext, const FactoryDefinition* pFactory)
{
    if (!pFactory->HasOwnOutputPipe() || m_outputItems.IsEmpty())
    {
        return;
    }

    Vector3 pipe = {
        pFactory->GetOutputPipe().GetXY().Rotate(rotationFromDirection(m_direction), m_size.GetXY()),
        pFactory->GetOutputPipe().GetZ()
    };
    pipe += m_position;

    Entity* pEntity = kContext.m_rMap.GetEntity(grid::getForwardPosition(pipe, m_direction));
    if (!pEntity || !pEntity->SupportsInsertion())
    {
        return;
    }

    auto& vContainerItems = m_outputItems.GetItems();
    auto itItems = vContainerItems.begin();
    while (itItems != vContainerItems.end())
    {
        for (uint32_t i = 0; i < itItems->m_pCount; ++i)
        {
            if (!pEntity->TryInsert(kContext, *this, InsertInfo(itItems->m_pItem, (m_uiTick + i) % c_conveyorChannels)))
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

bool cpp_conv::Factory::TrySatisfyRecipeInput(const FactoryDefinition* pFactory, uint64_t& uiOutEffort)
{
    const resources::AssetPtr<RecipeDefinition> pRecipe = resources::getRecipeDefinition(m_hActiveRecipeId);
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
