#include "ConveyorRenderingSystem.h"
#include <array>
#include <iostream>
#include <ranges>

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
        const std::vector<ConveyorPosition> positions)
    {
        for(const auto& segment : model->GetMesh()->GetSegments())
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
    std::map<atlas::resource::BundleRegistryId, ConveyorInstanceSet> items;
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

        bool hasComponents = ecs.DoesEntityHaveComponents<cpp_conv::components::PositionComponent, cpp_conv::components::DirectionComponent>(sequence.m_HeadConveyor);
        assert(hasComponents);
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

                auto& itemSet = items[itemAsset->GetAssetId()];
                if (!itemSet.m_Model)
                {
                    itemSet.m_Model = atlas::resource::ResourceLoader::LoadAsset<atlas::render::ModelAsset>(itemAsset->GetAssetId());
                }

                Eigen::Vector2f position2d = cpp_conv::conveyor_helper::getSlotPosition(sequence, sequenceIndex, channel, sequenceSlot);
                if (itemSlot->m_bIsAnimated)
                {
                    position2d = itemSlot->m_PreviousVisualLocation + ((position2d - itemSlot->m_PreviousVisualLocation) * fLerpFactor);
                    Eigen::Vector3f finalPosition {position2d.x(), headPosition.y() + 0.1f, position2d.y()};
                    itemSet.m_ConveyorPositions.emplace_back(finalPosition, cpp_conv::math_helpers::Angle::FromRadians(0));
                }
                else
                {
                    Eigen::Vector3f finalPosition {position2d.x(), headPosition.y() + 0.1f, position2d.y()};
                    itemSet.m_ConveyorPositions.emplace_back(finalPosition, cpp_conv::math_helpers::Angle::FromRadians(0));
                }
            }
        }
    }

    for(const auto entity : ecs.GetEntitiesWithComponents<cpp_conv::components::ConveyorComponent, cpp_conv::components::IndividuallyProcessableConveyorComponent>())
    {
        const auto& conveyor = ecs.GetComponent<cpp_conv::components::ConveyorComponent>(entity);
        bool hasComponents = ecs.DoesEntityHaveComponents<cpp_conv::components::PositionComponent, cpp_conv::components::DirectionComponent>(entity);
        assert(hasComponents);

        auto [position, direction] = ecs.GetComponents<
            cpp_conv::components::PositionComponent,
            cpp_conv::components::DirectionComponent>(entity);

        const ConveyorType type = conveyor.m_bIsClockwise ? Clockwise : AntiClockwise;
        conveyors[type].m_ConveyorPositions.emplace_back(
                            (position.m_Position).cast<float>(),
                            cpp_conv::rotationRadiansFromDirection(direction.m_Direction));

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

                auto& itemSet = items[itemAsset->GetAssetId()];
                if (!itemSet.m_Model)
                {
                    itemSet.m_Model = atlas::resource::ResourceLoader::LoadAsset<atlas::render::ModelAsset>(itemAsset->GetAssetId());
                }

                const auto& rTargetChannel = conveyor.m_Channels[channel];
                Eigen::Vector2f position2d = rTargetChannel.m_pSlots[slot].m_VisualPosition;
                if (itemSlot->m_bIsAnimated)
                {
                    position2d = itemSlot->m_PreviousVisualLocation + ((position2d - itemSlot->m_PreviousVisualLocation) * fLerpFactor);
                    Eigen::Vector3f finalPosition {position2d.x(), position.m_Position.y() + 0.1f, position2d.y()};
                    itemSet.m_ConveyorPositions.emplace_back(finalPosition, cpp_conv::math_helpers::Angle::FromRadians(0));
                }
                else
                {
                    Eigen::Vector3f finalPosition {position2d.x(), position.m_Position.y() + 0.1f, position2d.y()};
                    itemSet.m_ConveyorPositions.emplace_back(finalPosition, cpp_conv::math_helpers::Angle::FromRadians(0));
                }
            }
        }
    }

    atlas::render::addToFrameGraph_oneOff("ConveyorRenderer", [conveyors, items]()
    {
        const bool instancingSupported = 0 != (BGFX_CAPS_INSTANCING & bgfx::getCaps()->supported);
        for(auto& conveyorType : conveyors)
        {
            if (conveyorType.m_ConveyorPositions.empty())
            {
                continue;
            }

            if (instancingSupported)
            {
                drawInstanced(conveyorType.m_Model, conveyorType.m_Model->GetProgram(), conveyorType.m_ConveyorPositions);
            }
            else
            {
                // We don't currently support non-instanced platforms
                assert(false);
                drawNonInstanced(conveyorType.m_Model, conveyorType.m_Model->GetProgram(), conveyorType.m_ConveyorPositions);
            }
        }

        for(auto& item : items | std::ranges::views::values)
        {
            if (item.m_ConveyorPositions.empty())
            {
                continue;
            }

            if (instancingSupported)
            {
                drawInstanced(item.m_Model, item.m_Model->GetProgram(), item.m_ConveyorPositions);
            }
            else
            {
                // We don't currently support non-instanced platforms
                assert(false);
                drawNonInstanced(item.m_Model, item.m_Model->GetProgram(), item.m_ConveyorPositions);
            }
        }
    });
}
