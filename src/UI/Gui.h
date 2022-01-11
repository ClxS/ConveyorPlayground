#include "Colour.h"
#include "RenderContext.h"
#include <tuple>

uint32_t operator ""_Lines(unsigned long long input);

namespace cpp_conv::ui
{
    enum NumParts
    {
        ZeroX0      = 0b00000001,
        ZeroY0      = 0b00000010,
        MaxX1       = 0b00000100,
        MaxY1       = 0b00001000,
        ValueX1     = 0b00010000,
        ValueY1     = 0b00100000,
        NValueX1    = 0b01000000,
        NValueY1    = 0b10000000,
    };

    enum class Align
    {
        Stretch     = ZeroX0 | ZeroY0 | MaxX1 | MaxY1,
        Left        = ZeroX0 | ZeroY0 | ValueX1 | MaxY1,
        Right       = NValueX1 | ZeroY0 | MaxX1 | MaxY1,
        Top         = ZeroX0 | ZeroY0 | MaxX1 | ValueY1,
        Bottom      = ZeroX0 | NValueY1 | MaxX1 | MaxY1,
        TopLeft     = ZeroX0 | ZeroY0 | ValueX1 | ValueY1,
        TopRight    = NValueX1 | ZeroY0 | MaxX1 | ValueY1,
        BottomLeft  = ZeroX0 | NValueY1 | ValueX1 | MaxY1,
        BottomRight = NValueX1 | NValueY1 | MaxX1 | MaxY1,
    };

    namespace platform
    {
        void drawText(const std::string& szText, Colour colour, uint32_t x, uint32_t y);
        void drawWrappedText(const std::string& szText, Colour colour, uint32_t x, uint32_t y, uint32_t& linesRequired);

        uint32_t getTextLineHeight();
    }

    void setContext(cpp_conv::RenderContext* pContext);

    void initializeGuiSystem(uint32_t designWidth, uint32_t designHeight);

    std::tuple<uint32_t, uint32_t> getDesignDimensions();
    cpp_conv::RenderContext* getCurrentContext();


    void panel(const char* szIdentifier, Align panelAlignment = Align::Stretch, uint32_t uiWidth = 0, uint32_t uiHeight = 0, bool bInvertPlacement = false);

    void endPanel();

    void text(const std::string& szText, Colour colour = { 0x00FFFFFF });
    void wrappedText(const std::string& szText, Colour colour = { 0x00FFFFFF });
}
