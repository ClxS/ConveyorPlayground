#pragma once
#include "AtlasScene/ECS/Components/ComponentRegistry.h"

struct TransformComponent
{
    TransformComponent(): m_X{0}
    {
    }

    explicit TransformComponent(const int x)
        : m_X(x)
    {
    }

    int m_X;
};

struct SizeComponent
{
    SizeComponent(): m_X{0}
    {
    }

    explicit SizeComponent(const int x)
        : m_X(x)
    {
    }

    int m_X;
};

struct TestComponent
{
};

inline void registerTestComponents()
{
    atlas::scene::ComponentRegistry::RegisterComponent<TransformComponent>();
    atlas::scene::ComponentRegistry::RegisterComponent<SizeComponent>();
    atlas::scene::ComponentRegistry::RegisterComponent<TestComponent>();
}
