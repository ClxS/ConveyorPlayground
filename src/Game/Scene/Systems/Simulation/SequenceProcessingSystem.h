#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class EntityLookupGrid;

    class SequenceProcessingSystem_Process final : public atlas::scene::SystemBase
    {
    public:
        explicit SequenceProcessingSystem_Process(EntityLookupGrid& lookupGrid);

        void Update(atlas::scene::EcsManager&) override;

    private:
        EntityLookupGrid& m_LookupGrid;
    };

    class SequenceProcessingSystem_Realize final : public atlas::scene::SystemBase
    {
    public:
        void Update(atlas::scene::EcsManager&) override;
    };
}
