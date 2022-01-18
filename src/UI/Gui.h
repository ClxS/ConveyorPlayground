#include "Colour.h"
#include "RenderContext.h"
#include <tuple>

namespace cpp_conv::ui
{
    void initializeGuiSystem();
    void newFrame();
    void present();
    void shutdown();

    void drawUI(SceneContext& kContext, RenderContext& kRenderContext);
}
