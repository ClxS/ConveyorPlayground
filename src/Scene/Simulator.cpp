#include "Simulator.h"
#include "SceneContext.h"

void cpp_conv::simulation::simulate(SceneContext& kContext)
{
    for (auto& sequence : kContext.m_sequences)
    {
        sequence.Tick(kContext); 
    }
    
    for (cpp_conv::Conveyor* pConveyor : kContext.m_conveyors)
    {
        pConveyor->Tick(kContext);        
    }

    for (cpp_conv::Entity* pProducer : kContext.m_vOtherEntities)
    {
        pProducer->Tick(kContext);
    }
}
