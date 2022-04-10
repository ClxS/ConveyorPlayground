#pragma once

#include "RenderContext.h"

namespace cpp_conv::ui
{
    void initializeGuiSystem();
    void newFrame();
    void present();
    void shutdown();

    //void drawUI(SceneContext& kSceneContext, const RenderContext& kRenderContext);
}
