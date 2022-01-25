#include "Simulator.h"
#include "SceneContext.h"
#include "WorldMap.h"

void cpp_conv::simulation::simulate(SceneContext& kContext)
{
    for (auto& sequence : kContext.m_sequences)
    {
        sequence->Tick(kContext); 
    }

    for (cpp_conv::Entity* pProducer : kContext.m_rMap.GetOtherEntities())
    { 
        pProducer->Tick(kContext);
    }
    
    for (cpp_conv::Conveyor* pConveyor : kContext.m_rMap.GetCornerConveyors())
    {
        pConveyor->Tick(kContext);
    }

    for (auto& sequence : kContext.m_sequences)
    {
        sequence->Realize();
    }

    for (cpp_conv::Conveyor* pConveyor : kContext.m_rMap.GetCornerConveyors())
    {
        pConveyor->Realize();
    }
}
