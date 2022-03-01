#include "Simulator.h"
#include "SceneContext.h"
#include "WorldMap.h"

void cpp_conv::simulation::simulate(const SceneContext& kContext)
{
    for (const auto& sequence : kContext.m_sequences)
    {
        sequence->Tick(kContext);
    }

    for (Conveyor* pConveyor : kContext.m_rMap.GetCornerConveyors())
    {
        pConveyor->Tick(kContext);
    }

    for (Entity* pEntity : kContext.m_rMap.GetOtherEntities())
    {
        pEntity->Tick(kContext);
    }

    for (const auto& sequence : kContext.m_sequences)
    {
        sequence->Realize();
    }

    for (Conveyor* pConveyor : kContext.m_rMap.GetCornerConveyors())
    {
        pConveyor->Realize();
    }

    for (Entity* pEntity : kContext.m_rMap.GetOtherEntities())
    {
        pEntity->Realize();
    }
}
