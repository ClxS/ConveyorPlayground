#include "Gui.h"
#include <vector>
#include <queue>
#include "RenderContext.h"

struct Coord
{
    uint32_t m_x;
    uint32_t m_y;
};

struct Panel
{
    Coord m_uiTopLeft;
    Coord m_uiBottomRight;

    bool m_bInvertPlacement;

    void MoveY(uint32_t distance)
    {
        if (m_uiTopLeft.m_y + distance > m_uiBottomRight.m_y)
        {
            return;
        }

        if (m_bInvertPlacement)
        {
            if (m_uiBottomRight.m_y < distance)
            {
                m_uiBottomRight.m_y = 0;
            }
            else
            {
                m_uiBottomRight.m_y -= distance;
            }
        }
        else
        {
            m_uiTopLeft.m_y += distance;
        }
    }
};

static uint32_t g_designWidth;
static uint32_t g_designHeight;
static cpp_conv::RenderContext* g_pContext;
std::queue<Panel> g_panelStack;

void cpp_conv::ui::setContext(cpp_conv::RenderContext* pContext)
{
    g_pContext = pContext;
    while (!g_panelStack.empty())
    {
        g_panelStack.pop();
    }

    g_panelStack.push({ { 0, 0 }, { g_designWidth, g_designHeight } });
}

void cpp_conv::ui::initializeGuiSystem(uint32_t designWidth, uint32_t designHeight)
{
    g_designWidth = designWidth;
    g_designHeight = designHeight;
}

void cpp_conv::ui::panel(const char* szIdentifier, Align panelAlignment, uint32_t uiWidth, uint32_t uiHeight, bool bInvertPlacement)
{
    Panel newPanel = { {}, {}, bInvertPlacement };

    const Panel& rPanel = g_panelStack.back();

    if (((uint32_t)panelAlignment & ZeroX0) != 0) newPanel.m_uiTopLeft.m_x = 0;
    if (((uint32_t)panelAlignment & ZeroY0) != 0) newPanel.m_uiTopLeft.m_y = 0;
    if (((uint32_t)panelAlignment & NValueX1) != 0) newPanel.m_uiTopLeft.m_x = rPanel.m_uiBottomRight.m_x - uiWidth;
    if (((uint32_t)panelAlignment & NValueY1) != 0) newPanel.m_uiTopLeft.m_y = rPanel.m_uiBottomRight.m_y - uiHeight;
    if (((uint32_t)panelAlignment & MaxX1) != 0) newPanel.m_uiBottomRight.m_x = rPanel.m_uiBottomRight.m_x;
    if (((uint32_t)panelAlignment & MaxY1) != 0) newPanel.m_uiBottomRight.m_y = rPanel.m_uiBottomRight.m_y;
    if (((uint32_t)panelAlignment & ValueX1) != 0) newPanel.m_uiBottomRight.m_x = uiWidth;
    if (((uint32_t)panelAlignment & ValueY1) != 0) newPanel.m_uiBottomRight.m_y = uiHeight;

    g_panelStack.push(newPanel);
}

void cpp_conv::ui::endPanel()
{
    g_panelStack.pop();
}

void cpp_conv::ui::text(const std::string& szText, Colour colour /*= { 0xFFFFFFFF }*/)
{
    Panel& rPanel = g_panelStack.back();

    cpp_conv::ui::platform::drawText(
        szText,
        colour,
        rPanel.m_uiTopLeft.m_x,
        rPanel.m_bInvertPlacement
            ? (rPanel.m_uiBottomRight.m_y)
            : rPanel.m_uiTopLeft.m_y);
    rPanel.MoveY(cpp_conv::ui::platform::getTextLineHeight());
}

void cpp_conv::ui::wrappedText(const std::string& szText, Colour colour /*= { 0xFFFFFFFF }*/)
{
    Panel& rPanel = g_panelStack.back();

    uint32_t uiLinesRequired = 0;
    cpp_conv::ui::platform::drawWrappedText(szText, colour, rPanel.m_uiTopLeft.m_x, rPanel.m_uiTopLeft.m_y, uiLinesRequired);
    rPanel.MoveY(cpp_conv::ui::platform::getTextLineHeight());
}

std::tuple<uint32_t, uint32_t> cpp_conv::ui::getDesignDimensions()
{
    return std::make_tuple(g_designWidth, g_designHeight);
}

cpp_conv::RenderContext* cpp_conv::ui::getCurrentContext()
{
    return g_pContext;
}
