#include "Simulator.h"
#include "SceneContext.h"
#include "WorldMap.h"

void cpp_conv::simulation::simulate(SceneContext& kContext)
{
    for (const auto& sequence : kContext.m_sequences)
    {
        sequence->Tick(kContext); 
    }

    for (Entity* pProducer : kContext.m_rMap.GetOtherEntities())
    { 
        pProducer->Tick(kContext);
    }
    
    for (Conveyor* pConveyor : kContext.m_rMap.GetCornerConveyors())
    {
        pConveyor->Tick(kContext);
    }

    for (const auto& sequence : kContext.m_sequences)
    {
        sequence->Realize();
    }

    for (Conveyor* pConveyor : kContext.m_rMap.GetCornerConveyors())
    {
        pConveyor->Realize();
    }
}
