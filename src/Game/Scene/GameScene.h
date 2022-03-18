#pragma once

#include <AtlasScene/Scene.h>

#include "DirectionComponent.h"
#include "PositionComponent.h"
#include "WorldMap.h"

namespace cpp_conv
{
    class GameScene : public atlas::scene::EcsScene
    {
    public:
        explicit GameScene(cpp_conv::WorldMap map)
            : m_InitialisationData{std::move(map)}
        {

        }

        void OnEntered() override
        {
            //atlas::scene::EcsManager& ecs = GetEcsManager();
            //for(auto& entity : m_InitialisationData.m_Map.GetConveyors())
            //{
                //auto ecsEntity = ecs.AddEntity();
                //ecs.AddComponent<components::PositionComponent>(ecsEntity, entity->m_position);
                //ecs.AddComponent<components::DirectionComponent>(ecsEntity, entity->m_direction);
            //}
        }
        void OnUpdate() override
        {

        }
        void OnRender() override
        {

        }
        void OnExited() override
        {

        }

    private:
        struct InitialisationData
        {
            cpp_conv::WorldMap m_Map;
        } m_InitialisationData;
    };
}
