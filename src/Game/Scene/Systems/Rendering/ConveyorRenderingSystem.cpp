#include "ConveyorRenderingSystem.h"
#include <array>
#include <iostream>

#include "Constants.h"
#include "ConveyorComponent.h"
#include "ConveyorHelper.h"
#include "DirectionComponent.h"
#include "ItemDefinition.h"
#include "ItemRegistry.h"
#include "PositionComponent.h"
#include "RenderContext.h"
#include "SDL_mouse.h"
#include "SequenceComponent.h"
#include "TileRenderHandler.h"
#include "AtlasRender/Renderer.h"
#include "AtlasRender/AssetTypes/ModelAsset.h"
#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/ResourceLoader.h"
#include "bgfx/bgfx.h"
#include "bx/bx.h"
#include "bx/hash.h"
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

namespace bgfx
{
    extern const char* getAttribName(Attrib::Enum _attr);
}

namespace
{
    struct ConveyorPosition
    {
        Eigen::Vector3f m_Position;
        cpp_conv::math_helpers::Angle m_Rotation;
    };

    void drawNonInstanced(
        const atlas::resource::AssetPtr<atlas::render::ModelAsset>& model,
        const atlas::resource::AssetPtr<atlas::render::ShaderProgram>& program,
        const atlas::render::MeshSegment& segment,
        const std::vector<ConveyorPosition> positions)
    {
        for(auto& [position, rotation]: positions)
        {
            Eigen::Affine3f t{Eigen::Translation3f(position.x(), position.y(), position.z())};
            Eigen::Affine3f r{Eigen::AngleAxisf(rotation.AsRadians(), Eigen::Vector3f(0, 1, 0))};
            Eigen::Matrix4f m = (t * r).matrix();
            bgfx::setTransform(m.data());

            setVertexBuffer(0, segment.m_VertexBuffer);
            setIndexBuffer(segment.m_IndexBuffer);

            int textureIndex = 0;
            for(const auto& texture : model->GetTextures())
            {
                setTexture(textureIndex++, texture.m_Sampler, texture.m_Texture->GetHandle());
            }

            submit(0, program->GetHandle());
        }
    }

    void drawInstanced(
        const atlas::resource::AssetPtr<atlas::render::ModelAsset>& model,
        const atlas::resource::AssetPtr<atlas::render::ShaderProgram>& program,
        const std::vector<ConveyorPosition> positions)
    {
        constexpr uint16_t instanceStride = sizeof(Eigen::Matrix4f);

        const uint32_t totalPositions = static_cast<uint32_t>(positions.size());
        const uint32_t numDrawableInstances = bgfx::getAvailInstanceDataBuffer(totalPositions, instanceStride);

        bgfx::InstanceDataBuffer idb;
        bgfx::allocInstanceDataBuffer(&idb, numDrawableInstances, instanceStride);

        // TODO Log how many entities couldn't be drawn
        for (uint32_t i = 0; i < numDrawableInstances; ++i)
        {
            auto& [position, rotation] = positions[i];
            Eigen::Affine3f t{Eigen::Translation3f(position.x(), position.y(), position.z())};
            Eigen::Affine3f r{Eigen::AngleAxisf(rotation.AsRadians(), Eigen::Vector3f(0, 1, 0))};
            Eigen::Matrix4f m = (t * r).matrix();

            std::memcpy(&(idb.data[i * instanceStride]), m.data(), instanceStride);
        }

        int textureIndex = 0;
        for(const auto& texture : model->GetTextures())
        {
            setTexture(textureIndex++, texture.m_Sampler, texture.m_Texture->GetHandle());
        }

        bgfx::setState(BGFX_STATE_DEFAULT);
        for(const auto& segment : model->GetMesh()->GetSegments())
        {
            setVertexBuffer(0, segment.m_VertexBuffer);
            setIndexBuffer(segment.m_IndexBuffer);
            setInstanceDataBuffer(&idb);
            submit(0, program->GetHandle());
        }
    }
}

struct LerpInformation
{
    Eigen::Vector2f m_PreviousPosition;
    float m_LerpFactor;
};


void ConveyorRenderingSystem::Update(atlas::scene::EcsManager& ecs)
{
    enum ConveyorType
    {
        Straight = 0,
        AntiClockwise = 1,
        Clockwise = 2,
        Max
    };

    struct ConveyorInstanceSet
    {
        atlas::resource::AssetPtr<atlas::render::ModelAsset> m_Model;
        std::vector<ConveyorPosition> m_ConveyorPositions;
    };

    std::vector<ConveyorInstanceSet> conveyors;
    conveyors.reserve(ConveyorType::Max);
    conveyors.emplace_back(atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::registry::CoreBundle, atlas::render::ModelAsset>(
    cpp_conv::resources::registry::core_bundle::assets::conveyors::models::c_ConveyorStraight));
    conveyors.emplace_back(atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::registry::CoreBundle, atlas::render::ModelAsset>(
    cpp_conv::resources::registry::core_bundle::assets::conveyors::models::c_ConveyorAntiClockwise));
    conveyors.emplace_back(atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::registry::CoreBundle, atlas::render::ModelAsset>(
            cpp_conv::resources::registry::core_bundle::assets::conveyors::models::c_ConveyorClockwise));

    for(const auto entity : ecs.GetEntitiesWithComponents<cpp_conv::components::SequenceComponent>())
    {
        const auto& sequence = ecs.GetComponent<cpp_conv::components::SequenceComponent>(entity);

        if (ecs.DoesEntityHaveComponents<cpp_conv::components::PositionComponent, cpp_conv::components::DirectionComponent>(sequence.m_HeadConveyor))
        {
            auto [headPositionComponent, direction] = ecs.GetComponents<
                cpp_conv::components::PositionComponent,
                cpp_conv::components::DirectionComponent>(sequence.m_HeadConveyor);
            Eigen::Vector3f headPosition = (headPositionComponent.m_Position).cast<float>();

            for(int conveyorSlot = 0; conveyorSlot < sequence.m_Length; ++conveyorSlot)
            {
                Eigen::Vector3f positionOffset = sequence.m_UnitDirection * (conveyorSlot);
                conveyors[ConveyorType::Straight].m_ConveyorPositions.emplace_back(
                    headPosition - positionOffset,
                    cpp_conv::rotationRadiansFromDirection(direction.m_Direction));
            }
        }

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
        if (ecs.DoesEntityHaveComponents<cpp_conv::components::PositionComponent, cpp_conv::components::DirectionComponent>(entity))
        {
            auto [position, direction] = ecs.GetComponents<
                cpp_conv::components::PositionComponent,
                cpp_conv::components::DirectionComponent>(entity);

            const ConveyorType type = conveyor.m_bIsClockwise ? Clockwise : AntiClockwise;
            conveyors[type].m_ConveyorPositions.emplace_back(
                                (position.m_Position).cast<float>(),
                                cpp_conv::rotationRadiansFromDirection(direction.m_Direction));
        }

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
                // drawItem(
                //    tileAsset.get(),
                //    rTargetChannel.m_pSlots[slot].m_VisualPosition,
                //    {{
                //        itemSlot->m_PreviousVisualLocation,
                //        fLerpFactor
                //    }});
            }
        }
    }

    atlas::render::addToFrameGraph_oneOff("ConveyorRenderer", [conveyors]()
    {
        if (conveyors.empty())
        {
            return;
        }

        const bool instancingSupported = 0 != (BGFX_CAPS_INSTANCING & bgfx::getCaps()->supported);
        for(auto& conveyorType : conveyors)
        {
            if (conveyorType.m_ConveyorPositions.empty())
            {
                continue;
            }

            const auto& model = conveyorType.m_Model;
            const auto& program = conveyorType.m_Model->GetProgram();


            if (instancingSupported)
            {
                drawInstanced(model, program, conveyorType.m_ConveyorPositions);
            }
            else
            {
                for(const auto& segment : model->GetMesh()->GetSegments())
                {
                    // We don't currently support non-instanced platforms
                    assert(false);
                    drawNonInstanced(model, program, segment, conveyorType.m_ConveyorPositions);
                }
            }
        }
    });
}
