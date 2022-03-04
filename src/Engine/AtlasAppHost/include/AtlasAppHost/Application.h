#pragma once

#include <tuple>

#include "AtlasAppHost/PlatformApplication.h"

namespace atlas::app_host
{
    class Application
    {
    public:
        Application(const Application&) = delete;
        Application(const Application&&) = delete;
        void operator=(const Application&) = delete;
        void operator=(const Application&&) = delete;

        static Application& Get()
        {
            static Application s_application;
            return s_application;
        }

        [[nodiscard]] bool Initialise() { return m_Platform.Initialise(); }
        [[nodiscard]] std::tuple<int, int> GetAppDimensions() const { return m_Platform.GetAppDimensions(); }

        [[nodiscard]] platform::PlatformApplication& GetPlatform() { return m_Platform; };
        [[nodiscard]] const platform::PlatformApplication& GetPlatform() const { return m_Platform; };

    private:
        Application() = default;
        ~Application() = default;

        platform::PlatformApplication m_Platform;
    };
}
