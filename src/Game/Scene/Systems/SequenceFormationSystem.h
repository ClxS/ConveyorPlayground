#pragma once
#include "EntityLookupGrid.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class SequenceFormationSystem : atlas::scene::SystemBase
    {
    public:
        explicit SequenceFormationSystem(EntityLookupGrid& lookupGrid)
            : m_LookupGrid{lookupGrid}
        {
        }

        void Initialise(atlas::scene::EcsManager& ecs) override;

        void Update(atlas::scene::EcsManager&) override
        {
        }

    private:
        EntityLookupGrid& m_LookupGrid;
    };
}
