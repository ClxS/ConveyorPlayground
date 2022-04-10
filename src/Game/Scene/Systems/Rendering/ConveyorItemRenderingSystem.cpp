#include "ConveyorItemRenderingSystem.h"
#include <array>

#include "ConveyorComponent.h"
#include "ConveyorHelper.h"
#include "ItemDefinition.h"
#include "ItemRegistry.h"
#include "RenderContext.h"
#include "SDL_mouse.h"
#include "SequenceComponent.h"
#include "TileRenderHandler.h"
#include "AtlasRender/Renderer.h"
#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/ResourceLoader.h"
#include "bgfx/bgfx.h"
#include "bx/bx.h"
#include "bx/math.h"

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
        static int prev_mouse_x = 0;
        static int prev_mouse_y = 0;
        static float cam_pitch = 0.0f;
        static float cam_yaw = 0.0f;
        static float rot_scale = 0.01f;
        static const int width = 800;
        static const int height = 600;

        struct PosColorVertex
        {
            float x;
            float y;
            float z;
            uint32_t abgr;
        };

        static PosColorVertex cube_vertices[] = {
            {-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
            {-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
            {-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
            {-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
        };

        static const uint16_t cube_tri_list[] = {
            0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6,
            1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7,
        };

        static bgfx::VertexBufferHandle vbh{};
        static bgfx::IndexBufferHandle ibh{};
        if (vbh.idx == 0)
        {
            bgfx::VertexLayout pos_col_vert_layout;
            pos_col_vert_layout.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                .end();
            vbh = bgfx::createVertexBuffer(
                bgfx::makeRef(cube_vertices, sizeof(cube_vertices)),
                pos_col_vert_layout);
            ibh = bgfx::createIndexBuffer(
                bgfx::makeRef(cube_tri_list, sizeof(cube_tri_list)));
        }


        auto program = atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::registry::CoreBundle, atlas::render::ShaderProgram>(
            cpp_conv::resources::registry::core_bundle::shaders::c_basic);

        int mouse_x, mouse_y;
           const int buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
           if ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0) {
               int delta_x = mouse_x - prev_mouse_x;
               int delta_y = mouse_y - prev_mouse_y;
               cam_yaw += float(-delta_x) * rot_scale;
               cam_pitch += float(-delta_y) * rot_scale;
           }

           prev_mouse_x = mouse_x;
           prev_mouse_y = mouse_y;

           float cam_rotation[16];
           bx::mtxRotateXYZ(cam_rotation, cam_pitch, cam_yaw, 0.0f);

           float cam_translation[16];
           bx::mtxTranslate(cam_translation, 0.0f, 0.0f, -5.0f);

           float cam_transform[16];
           bx::mtxMul(cam_transform, cam_translation, cam_rotation);

           float view[16];
           bx::mtxInverse(view, cam_transform);

           float proj[16];
           bx::mtxProj(
               proj, 60.0f, float(width) / float(height), 0.1f,
               100.0f, bgfx::getCaps()->homogeneousDepth);

           bgfx::setViewTransform(0, view, proj);

           float model[16];
           bx::mtxIdentity(model);
           bgfx::setTransform(model);

           bgfx::setVertexBuffer(0, vbh);
           bgfx::setIndexBuffer(ibh);

           bgfx::submit(0, program->GetHandle());
    });
}
