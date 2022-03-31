#pragma once
#include "EntityLookupGrid.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class SequenceFormationSystem final : public atlas::scene::SystemBase
    {
    public:
        explicit SequenceFormationSystem(EntityLookupGrid& lookupGrid);
        void Initialise(atlas::scene::EcsManager& ecs) override;
        void Update(atlas::scene::EcsManager&) override;

    private:
        inline static constexpr int c_MaxSequenceLength = 31;

        EntityLookupGrid& m_LookupGrid;
    };
}
