#pragma once

namespace atlas::scene
{
    class EcsManager;

    class SystemBase
    {
    public:
        virtual ~SystemBase() = default;

        virtual void Initialise(EcsManager&)
        {
        }

        virtual void Update(EcsManager&) = 0;

        virtual void Render(EcsManager&)
        {
        }
    };
}
