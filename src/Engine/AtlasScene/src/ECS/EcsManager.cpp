#include "AtlasScenePCH.h"
#include "AtlasScene/ECS/EcsManager.h"

#include <AsyncInfo.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Iterator
/*
atlas::scene::EcsManager::EntityComponentIterator::Iterator::Iterator(const EntityPool& entityPool,
    const uint64_t componentMask, const bool bIsEnd): m_EntityPool{entityPool}
                                                      , m_ComponentMask{componentMask}
                                                      , m_CurrentIndex(bIsEnd ? -1 : FindNextMatchingSlot(-1))
{
}

atlas::scene::EcsManager::EntityComponentIterator::Iterator::reference atlas::scene::EcsManager::EntityComponentIterator
::Iterator::operator*() const
{ return m_EntityPool.GetCopy(m_CurrentIndex).m_EntityId; }

atlas::scene::EcsManager::EntityComponentIterator::Iterator::pointer atlas::scene::EcsManager::EntityComponentIterator::
Iterator::operator->() const
{ return m_EntityPool.GetCopy(m_CurrentIndex).m_EntityId; }

atlas::scene::EcsManager::EntityComponentIterator::Iterator& atlas::scene::EcsManager::EntityComponentIterator::Iterator
::operator++()
{
    m_CurrentIndex = FindNextMatchingSlot(m_CurrentIndex);
    return *this;
}

atlas::scene::EcsManager::EntityComponentIterator::Iterator atlas::scene::EcsManager::EntityComponentIterator::Iterator
::operator++(int)
{ Iterator tmp = *this; ++(*this); return tmp; }

bool atlas::scene::operator==(const EcsManager::EntityComponentIterator::Iterator& a,
    const EcsManager::EntityComponentIterator::Iterator& b)
{ return a.m_CurrentIndex == b.m_CurrentIndex; }

bool atlas::scene::operator!=(const EcsManager::EntityComponentIterator::Iterator& a,
    const EcsManager::EntityComponentIterator::Iterator& b)
{ return a.m_CurrentIndex != b.m_CurrentIndex; }

int32_t atlas::scene::EcsManager::EntityComponentIterator::Iterator::FindNextMatchingSlot(const int32_t current) const
{
    for(int i = current + 1; i < m_EntityPool.Size(); ++i)
    {
        if ((m_EntityPool.GetCopy(i).m_PossessedComponentMask & m_ComponentMask) == m_ComponentMask)
        {
            return i;
        }
    }

    return -1;
}

atlas::scene::EcsManager::EntityComponentIterator::EntityComponentIterator(const EntityPool& entityPool,
    const uint64_t componentMask): m_EntityPool{entityPool}
                                   , m_ComponentMask{componentMask}
{
}*/

atlas::scene::EcsManager::EcsManager()
{
    m_ArchetypePools.emplace_back(0);
}

atlas::scene::EcsManager::~EcsManager()
{
    for(auto& [_1, _2, components] : m_ArchetypePools)
    {
        for(auto [_, pool] : components)
        {
            delete pool;
        }

        components.clear();
    }

    m_ArchetypePools.clear();
}

atlas::scene::EntityId atlas::scene::EcsManager::AddEntity()
{
    const EntityId id { m_EntityIndices.Size() };
    ArchetypeIndex archetypeIndex = ArchetypeIndex::Empty();
    auto& [_1, pool, _2] = GetPool(archetypeIndex);

    m_EntityIndices.Push(pool.Size(), archetypeIndex);
    pool.Push(id);
    return id;
}

void atlas::scene::EcsManager::RemoveEntity(const EntityId entity)
{
    const auto removedIndex = m_EntityIndices.GetCopy(entity.m_Value);
    auto& [_, entityPool, components] = GetPool(removedIndex.m_ArchetypeIndex);

    if (entityPool.Size() > 1)
    {
        const auto endEntityId = entityPool.GetCopy(entityPool.Size() - 1);
        if (endEntityId != entity)
        {
            m_EntityIndices.Remove(entity.m_Value);
            m_EntityIndices.Set(endEntityId.m_Value, removedIndex);
            entityPool.SwapAndPop(removedIndex.m_EntityIndex);
            for(const auto& [poolMask, pool] : components)
            {
                pool->SwapAndPop(removedIndex.m_EntityIndex);
            }
        }
        else
        {
            m_EntityIndices.Remove(entity.m_Value);
            entityPool.Pop();
            for(const auto& [_, pool] : components)
            {
                pool->Pop();
            }
        }
    }
    else
    {
        m_EntityIndices.Remove(entity.m_Value);
        entityPool.Pop();
        for(const auto& [_, pool] : components)
        {
            pool->Pop();
        }
    }
}

atlas::scene::ArchetypeIndex atlas::scene::EcsManager::GetOrCreateArchetype(uint64_t archetypeMask)
{
    for(int i = 0; i < m_ArchetypePools.size(); ++i)
    {
        if (m_ArchetypePools[i].m_ArchetypeComponentMask == archetypeMask)
        {
            return ArchetypeIndex{i};
        }
    }

    m_ArchetypePools.emplace_back(archetypeMask);
    auto& [mask, entityPool, componentPools] = m_ArchetypePools.back();

    while(archetypeMask > 0)
    {
        const uint64_t componentMask = (1ULL <<  std::countr_zero(archetypeMask));
        archetypeMask &= ~componentMask;

        componentPools.emplace_back(componentMask, ComponentRegistry::GetFactoryForPoolWithMask(componentMask)());
    }

    return ArchetypeIndex{static_cast<int32_t>(m_ArchetypePools.size()) - 1};
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ECS Manager
