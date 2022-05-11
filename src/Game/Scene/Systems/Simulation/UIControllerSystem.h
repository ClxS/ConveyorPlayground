#pragma once
#include <functional>
#include <memory>

#include "AtlasResource/AssetPtr.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

class RmlUIPage;

namespace Rml
{
    class Context;
    class FileInterface;
    class RenderInterface;
    class SystemInterface;
}

namespace cpp_conv
{
    class UIControllerSystem final : public atlas::scene::SystemBase
    {
    public:
        UIControllerSystem();
        ~UIControllerSystem() override;

        void Initialise(atlas::scene::EcsManager&) override;

        void Update(atlas::scene::EcsManager&) override;

    private:
        std::unique_ptr<Rml::FileInterface> m_FileInterface;
        std::unique_ptr<Rml::SystemInterface> m_SystemInterface;
        std::unique_ptr<Rml::RenderInterface> m_RenderInterface;
        std::unique_ptr<Rml::Context, std::function<void(Rml::Context*)>> m_Context;

        std::vector<atlas::resource::AssetPtr<RmlUIPage>> m_Pages;
    };
}
