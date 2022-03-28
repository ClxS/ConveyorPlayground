#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class EntityLookupGrid;

    class FactorySystem final : public atlas::scene::SystemBase
    {
    public:
        explicit FactorySystem(EntityLookupGrid& lookupGrid)
            : m_LookupGrid{lookupGrid}
        {
        }

        void Update(atlas::scene::EcsManager&) override;

    private:
        EntityLookupGrid& m_LookupGrid;

    };
}
