#pragma once

#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class EntityLookupGrid;

    class ConveyorStateDeterminationSystem final : public atlas::scene::SystemBase
    {
    public:
        explicit ConveyorStateDeterminationSystem(EntityLookupGrid& lookupGrid);
        void Initialise(atlas::scene::EcsManager& ecs) override;
        void Update(atlas::scene::EcsManager&) override;

    private:
        inline static constexpr int c_MaxSequenceLength = 32;

        EntityLookupGrid& m_LookupGrid;
    };
}
