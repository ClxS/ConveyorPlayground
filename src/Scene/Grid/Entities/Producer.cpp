#include "Producer.h"
#include "Conveyor.h"
#include "SceneContext.h"

#include "Renderer.h"
#include "RenderContext.h"

#include <array>
#include <random>

cpp_conv::Producer::Producer(int x, int y, Direction direction, Item* pItem, uint64_t productionRate)
    : Entity(x, y, EntityKind::Producer)
    , m_pItem(pItem)
    , m_direction(direction)
    , m_uiTick(0)
    , m_productionRate(productionRate)
    , m_bProductionReady(false)
{
}

bool cpp_conv::Producer::IsReadyToProduce() const
{
    return m_bProductionReady;
}

cpp_conv::Item* cpp_conv::Producer::ProduceItem()
{
    if (!m_bProductionReady)
    {
        return nullptr;
    }

    m_bProductionReady = false;
    return m_pItem;
}

void cpp_conv::Producer::Tick(const SceneContext& kContext)
{
    if (m_bProductionReady)
    {
        return;
    }

    m_uiTick++;
    if ((m_uiTick % m_productionRate) == 0)
    {
        m_bProductionReady = true;
    }

    if (!IsReadyToProduce())
    {
        return;
    }

    Item* pItem = ProduceItem();
    if (!pItem)
    {
        return;
    }

    cpp_conv::Entity* pForwardEntity = cpp_conv::grid::SafeGetEntity(kContext.m_grid, cpp_conv::grid::GetForwardPosition(*this, GetDirection()));
    if (!pForwardEntity || pForwardEntity->m_eEntityKind != EntityKind::Conveyor)
    {
        return;
    }

    std::array<int, cpp_conv::c_conveyorChannels> arrDirectionEntities;
    for (int arrChannelIdx = 0; arrChannelIdx < cpp_conv::c_conveyorChannels; ++arrChannelIdx)
    {
        arrDirectionEntities[arrChannelIdx] = arrChannelIdx;
    }

    std::default_random_engine engine(m_uiTick % 256);
    for (auto i = (arrDirectionEntities.end() - arrDirectionEntities.begin()) - 1; i > 0; --i)
    {
        std::uniform_int_distribution<decltype(i)> d(0, i);
        std::swap(arrDirectionEntities.begin()[i], arrDirectionEntities.begin()[d(engine)]);
    }

    bool bProduced = false;
    for (int iChannel : arrDirectionEntities)
    {
        cpp_conv::Conveyor* pConveyor = reinterpret_cast<cpp_conv::Conveyor*>(pForwardEntity);
        Item*& forwardTargetItem = pConveyor->m_pChannels[iChannel].m_pItems[0];
        Item*& forwardPendingItem = pConveyor->m_pChannels[iChannel].m_pItems[0];
        if (!forwardTargetItem && !forwardPendingItem)
        {
            forwardPendingItem = pItem;
            bProduced = true;
            break;
        }
    }

    if (!bProduced)
    {
        m_pItem = pItem;
        m_bProductionReady = true;
    }
}

void cpp_conv::Producer::Draw(RenderContext& kContext) const
{
    wchar_t character = L' ';
    switch (m_direction)
    {
    case Direction::Left:
        character = L'←';
        break;
    case Direction::Up:
        character = L'↑';
        break;
    case Direction::Right:
        character = L'→';
        break;
    case Direction::Down:
        character = L'↓';
        break;
    }

    /*cpp_conv::renderer::setPixel(kContext, character, m_position.m_x * cpp_conv::renderer::c_gridScale + 1, m_position.m_y * cpp_conv::renderer::c_gridScale + 1, 1, true);
    cpp_conv::renderer::setPixel(kContext, character, m_position.m_x * cpp_conv::renderer::c_gridScale + 2, m_position.m_y * cpp_conv::renderer::c_gridScale + 1, 1, true);
    cpp_conv::renderer::setPixel(kContext, character, m_position.m_x * cpp_conv::renderer::c_gridScale + 1, m_position.m_y * cpp_conv::renderer::c_gridScale + 2, 1, true);
    cpp_conv::renderer::setPixel(kContext, character, m_position.m_x * cpp_conv::renderer::c_gridScale + 2, m_position.m_y * cpp_conv::renderer::c_gridScale + 2, 1, true);*/
}
