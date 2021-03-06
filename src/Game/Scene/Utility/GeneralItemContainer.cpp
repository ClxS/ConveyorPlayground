#include "GeneralItemContainer.h"
#include <map>
#include "AtlasResource/AssetPtr.h"
#include "DataId.h"
#include "ItemDefinition.h"
#include "ItemRegistry.h"
#include "Profiler.h"

cpp_conv::GeneralItemContainer::GeneralItemContainer(
    const uint32_t uiMaxCapacity,
    const uint32_t uiMaxStackSize,
    const bool bUniqueStacksOnly)
    : m_uiMaxCapacity(uiMaxCapacity)
      , m_uiMaxStackSize(uiMaxStackSize)
      , m_bUniqueStacksOnly(bUniqueStacksOnly)
{
}

void cpp_conv::GeneralItemContainer::Initialise(const uint32_t uiMaxCapacity, const uint32_t uiMaxStackSize, const bool bUniqueStacksOnly)
{
    m_uiMaxCapacity = uiMaxCapacity;
    m_uiMaxStackSize = uiMaxStackSize;
    m_bUniqueStacksOnly = bUniqueStacksOnly;
}

bool cpp_conv::GeneralItemContainer::TryTake(bool bSingle, std::tuple<ItemId, uint32_t>& outItem)
{
    if (m_vItemEntries.empty())
    {
        return false;
    }

    uint32_t uiCount = 1;
    if (bSingle)
    {
        uiCount = m_vItemEntries[0].m_pCount;
    }

    outItem = std::make_tuple(m_vItemEntries[0].m_pItem, m_vItemEntries[0].m_pCount);

    m_vItemEntries[0].m_pCount -= uiCount;
    if (m_vItemEntries[0].m_pCount == 0)
    {
        m_vItemEntries.erase(m_vItemEntries.begin());
    }

    return true;
}

bool cpp_conv::GeneralItemContainer::TryTake(ItemId item, uint32_t count /*= 1*/)
{
    if (!HasItems(item, count))
    {
        return false;
    }

    auto itItems = m_vItemEntries.begin();
    while (itItems != m_vItemEntries.end())
    {
        if (itItems->m_pItem != item)
        {
            ++itItems;
            continue;
        }

        if (itItems->m_pCount >= count)
        {
            itItems->m_pCount -= count;
            return true;
        }

        count -= itItems->m_pCount;
        itItems = m_vItemEntries.erase(itItems);
    }

    return false;
}

bool cpp_conv::GeneralItemContainer::TryInsert(ItemId pItem, uint32_t count)
{
    PROFILE_FUNC();
    if (!CouldInsert(pItem, count))
    {
        return false;
    }

    for (auto& rItemEntry : m_vItemEntries)
    {
        if (rItemEntry.m_pItem == pItem)
        {
            if ((rItemEntry.m_pCount + count) < m_uiMaxStackSize)
            {
                rItemEntry.m_pCount += count;
                return true;
            }

            const uint32_t uiSpace = m_uiMaxStackSize - rItemEntry.m_pCount;
            count -= uiSpace;
            rItemEntry.m_pCount += uiSpace;
        }
    }

    const uint32_t remainingFullStacks = count / m_uiMaxStackSize;
    uint32_t remainingPartialStack = count % m_uiMaxStackSize;

    for (uint32_t i = 0; i < remainingFullStacks; i++)
    {
        m_vItemEntries.emplace_back(pItem, m_uiMaxStackSize);
    }

    if (remainingPartialStack != 0)
    {
        m_vItemEntries.emplace_back(pItem, remainingPartialStack);
    }

    return true;
}

bool cpp_conv::GeneralItemContainer::CouldInsert(ItemId pItem, uint32_t count /*= 1*/)
{
    bool bIsMet = true;
    for (auto& rItem : m_vItemEntries)
    {
        if (rItem.m_pItem == pItem)
        {
            if ((rItem.m_pCount + count) > m_uiMaxStackSize)
            {
                if (m_bUniqueStacksOnly)
                {
                    bIsMet = false;
                    break;
                }

                count -= m_uiMaxStackSize - rItem.m_pCount;
            }
            else
            {
                return true;
            }
        }
    }

    if (!bIsMet)
    {
        return false;
    }

    const uint32_t extraSlotsRequired = count / m_uiMaxStackSize + ((count % m_uiMaxStackSize) == 0 ? 0 : 1);
    return (m_vItemEntries.size() + extraSlotsRequired) <= m_uiMaxCapacity;
}

bool cpp_conv::GeneralItemContainer::HasItems(ItemId item, uint32_t count)
{
    PROFILE_FUNC();
    for (auto& rItemEntry : m_vItemEntries)
    {
        if (rItemEntry.m_pItem != item)
        {
            continue;
        }

        if (rItemEntry.m_pCount >= count)
        {
            return true;
        }

        if (!m_bUniqueStacksOnly)
        {
            count -= rItemEntry.m_pCount;
        }
        else
        {
            return false;
        }
    }

    return false;
}

bool cpp_conv::GeneralItemContainer::IsEmpty() const
{
    return m_vItemEntries.empty();
}

std::string cpp_conv::GeneralItemContainer::GetDescription() const
{
    std::map<ItemId, int> storedItems;

    for (const ItemEntry& itemEntry : m_vItemEntries)
    {
        ItemId item = itemEntry.m_pItem;
        if (!item.IsEmpty())
        {
            storedItems.try_emplace(item, 0);
            storedItems[item] += itemEntry.m_pCount;
        }
    }

    std::string str = "";
    bool bFirst = true;
    if (!storedItems.empty())
    {
        for (auto& item : storedItems)
        {
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                str += ", ";
            }

            atlas::resource::AssetPtr<ItemDefinition> pItem = resources::getItemDefinition(item.first);
            if (pItem)
            {
                str += std::format("{} {}", item.second, pItem->GetName());
            }
            else
            {
                str += std::format("{} Unknown Items", item.second);
            }
        }
    }

    return std::format("{}/{} - {}", m_vItemEntries.size(), m_uiMaxCapacity, str);
}
