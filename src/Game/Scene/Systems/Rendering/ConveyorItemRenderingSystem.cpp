#include "ConveyorItemRenderingSystem.h"
#include <array>

#include "ConveyorComponent.h"
#include "ConveyorHelper.h"
#include "ItemDefinition.h"
#include "ItemRegistry.h"
#include "RenderContext.h"
#include "SequenceComponent.h"
#include "TileRenderHandler.h"
#include "AtlasRender/Renderer.h"
#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/ResourceLoader.h"
#include "bgfx/bgfx.h"

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

    /*tileRenderer(
        *g_renderContext,
        pTile,
        { renderPosition.x(), renderPosition.y() },
        {0xFFFFFFFF},
        true);*/
}

void ConveyorItemRenderingSystem::Update(atlas::scene::EcsManager& ecs)
{
    for(const auto entity : ecs.GetEntitiesWithComponents<cpp_conv::components::SequenceComponent>())
    {
        const auto& sequence = ecs.GetComponent<cpp_conv::components::SequenceComponent>(entity);

        const float fLerpFactor = sequence.m_CurrentTick / static_cast<float>(sequence.m_MoveTick);

        for(int channel = 0; channel < cpp_conv::components::c_conveyorChannels; channel++)
        {
            auto lanes = sequence.m_RealizedStates[channel].m_Lanes;
            if (sequence.m_RealizedStates[channel].m_Lanes == 0)
            {
                continue;
            }

            while (lanes != 0)
            {
                const auto nextItemBit = std::countr_zero(lanes);
                const auto nextSlot = (sequence.m_Length * 2) - nextItemBit - 1;
                lanes &= ~(1ULL << nextItemBit);

                const auto sequenceIndex = nextSlot / 2;
                const auto sequenceSlot = nextSlot % 2;

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
                const atlas::resource::AssetPtr<cpp_conv::ItemDefinition> itemAsset = cpp_conv::resources::getItemDefinition(itemSlot->m_Item);
                if (!itemAsset || !itemAsset->GetAssetId().IsValid())
                {
                    continue;
                }

                const auto tileAsset = atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::TileAsset>(itemAsset->GetAssetId());
                if (!tileAsset)
                {
                    continue;
                }

                if (itemSlot->m_bIsAnimated)
                {
                    /*drawItem(
                        tileAsset,
                        cpp_conv::conveyor_helper::getSlotPosition(sequence, sequenceIndex, channel, sequenceSlot),
                        {{
                            itemSlot->m_PreviousVisualLocation,
                            fLerpFactor
                        }});*/
                }
                else
                {
                    //drawItem(tileAsset,cpp_conv::conveyor_helper::getSlotPosition(sequence, sequenceIndex, channel, sequenceSlot));
                }
            }
        }
    }

    for(const auto entity : ecs.GetEntitiesWithComponents<cpp_conv::components::ConveyorComponent, cpp_conv::components::IndividuallyProcessableConveyorComponent>())
    {
        const auto& conveyor = ecs.GetComponent<cpp_conv::components::ConveyorComponent>(entity);
        const float fLerpFactor = conveyor.m_CurrentTick / static_cast<float>(conveyor.m_MoveTick);

        for (int channel = 0; channel < cpp_conv::components::c_conveyorChannels; channel++)
        {
            int iChannelLength = cpp_conv::components::c_conveyorChannelSlots;
            if (conveyor.m_bIsCorner)
            {
                iChannelLength += conveyor.m_InnerMostChannel == channel ? -1 : 1;
            }

            for (int slot = 0; slot < iChannelLength; slot++)
            {
                auto itemSlot = cpp_conv::conveyor_helper::getItemInSlot(
                    conveyor,
                    channel,
                    slot);

                if (!itemSlot.has_value())
                {
                    continue;
                }

                // TODO: This should be cached better, the constant item definition lookups get expensive
                const atlas::resource::AssetPtr<cpp_conv::ItemDefinition> itemAsset = cpp_conv::resources::getItemDefinition(itemSlot->m_Item);
                if (!itemAsset || !itemAsset->GetAssetId().IsValid())
                {
                    continue;
                }

                const auto tileAsset = atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::TileAsset>(itemAsset->GetAssetId());
                if (!tileAsset)
                {
                    continue;
                }

                const auto& rTargetChannel = conveyor.m_Channels[channel];
                drawItem(
                   tileAsset.get(),
                   rTargetChannel.m_pSlots[slot].m_VisualPosition,
                   {{
                       itemSlot->m_PreviousVisualLocation,
                       fLerpFactor
                   }});
            }
        }
    }

    atlas::render::addToFrameGraph_oneOff("ConveyorRenderer", []()
    {
        atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::registry::CoreBundle, atlas::render::ShaderProgram>(
            cpp_conv::resources::registry::core_bundle::shaders::c_basic);
        int i = 0;
        i++;
    });
}
