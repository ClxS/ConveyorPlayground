#include "ConveyorItemRenderingSystem.h"
#include <array>
#include <cstdint>

#include "AssetPtr.h"
#include "ConveyorComponent.h"
#include "ConveyorHelper.h"
#include "ItemDefinition.h"
#include "ItemRegistry.h"
#include "RenderContext.h"
#include "SequenceComponent.h"
#include "TargetingUtility.h"
#include "TileRenderHandler.h"

// TODO REMOVE THIS. This needs to go once we move to Atlas Render
extern cpp_conv::RenderContext* g_renderContext;

/*
*
            tileRenderer(
                *g_renderContext,
                sprite.m_pTile.get(),
                {static_cast<float>(position.m_Position.x()) * 4, static_cast<float>(position.m_Position.y()) * 4},
                {0xFFFFFFFF},
                true);
 */

struct LerpInformation
{
    Eigen::Vector2f m_PreviousPosition;
    float m_LerpFactor;
};

void drawItem(
    const cpp_conv::resources::TileAsset* pTile,
    Eigen::Vector2f renderPosition,
    const std::optional<LerpInformation> lerp = {})
{
    if (lerp.has_value())
    {
        renderPosition = lerp->m_PreviousPosition + ((renderPosition - lerp->m_PreviousPosition) * lerp->m_LerpFactor);
    }

    tileRenderer(
        *g_renderContext,
        pTile,
        { renderPosition.x(), renderPosition.y() },
        {0xFFFFFFFF},
        true);
}

void ConveyorItemRenderingSystem::Update(atlas::scene::EcsManager& ecs)
{
    for(const auto entity : ecs.GetEntitiesWithComponents<cpp_conv::components::SequenceComponent>())
    {
        const auto& sequence = ecs.GetComponent<cpp_conv::components::SequenceComponent>(entity);

        const float fLerpFactor = sequence.m_CurrentTick / static_cast<float>(sequence.m_MoveTick);

        for(int channel = 0; channel < cpp_conv::components::c_conveyorChannels; channel++)
        {
            for(int slot = 0; slot < sequence.m_Length * 2; slot++)
            {
                const auto sequenceIndex = slot / 2;
                const auto sequenceSlot = slot % 2;

                // Redundant since getItemInSlot will double check, but this will probably get inlined and be faster
                if (!cpp_conv::conveyor_helper::hasRealizedItemInSlot(sequence, sequenceIndex, channel, sequenceSlot))
                {
                    continue;
                }

                auto itemSlot = cpp_conv::conveyor_helper::getItemInSlot(
                    sequence,
                    sequenceIndex,
                    channel,
                    sequenceSlot);

                if (!itemSlot.has_value())
                {
                    continue;
                }

                // TODO: This should be cached better, the constant item definition lookups get expensive
                const cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition> itemAsset = cpp_conv::resources::getItemDefinition(itemSlot->m_Item);
                if (!itemAsset || !itemAsset->GetTile())
                {
                    continue;
                }

                const auto tileAsset = itemAsset->GetTile().get();
                if (itemSlot->m_bIsAnimated)
                {
                    drawItem(
                        tileAsset,
                        cpp_conv::conveyor_helper::getSlotPosition(sequence, sequenceIndex, channel, sequenceSlot),
                        {{
                            itemSlot->m_PreviousVisualLocation,
                            fLerpFactor
                        }});
                }
                else
                {
                    drawItem(tileAsset,cpp_conv::conveyor_helper::getSlotPosition(sequence, sequenceIndex, channel, sequenceSlot));
                }
            }
        }
    }
}
