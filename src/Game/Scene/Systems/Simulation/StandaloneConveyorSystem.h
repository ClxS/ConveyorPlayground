#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class EntityLookupGrid;

    class StandaloneConveyorSystem_Process final : public atlas::scene::SystemBase
    {
    public:
        explicit StandaloneConveyorSystem_Process(EntityLookupGrid& lookupGrid)
            : m_LookupGrid{lookupGrid}
        {
        }

        void Update(atlas::scene::EcsManager&) override;

    private:
        EntityLookupGrid& m_LookupGrid;
    };

    class StandaloneConveyorSystem_Realize final : public atlas::scene::SystemBase
    {
    public:
        void Update(atlas::scene::EcsManager&) override;
    };
}
