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

struct LerpInformation
{
    Eigen::Vector2f m_PreviousPosition;
    float m_LerpFactor;
};

void ConveyorRenderingSystem::Initialise(atlas::scene::EcsManager&, const uint8_t viewId)
{
    m_ViewId = viewId;
}

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
        std::vector<Eigen::Matrix4f> m_ConveyorPositions;
    };

    std::vector<ConveyorInstanceSet> conveyors;
    std::map<atlas::resource::BundleRegistryId, ConveyorInstanceSet> items;
    conveyors.reserve(Max);
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

            auto translation = headPosition - positionOffset;
            auto rotation = cpp_conv::rotationRadiansFromDirection(direction.m_Direction);
            Eigen::Affine3f t{Eigen::Translation3f(translation.x(), translation.y(), translation.z())};
            Eigen::Affine3f r{Eigen::AngleAxisf(rotation.AsRadians(), Eigen::Vector3f(0, 1, 0))};
            Eigen::Matrix4f m = (t * r).matrix();

            conveyors[Straight].m_ConveyorPositions.push_back(m);
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
                    Eigen::Affine3f t{Eigen::Translation3f(position2d.x(), headPosition.y() + 0.1f, position2d.y())};
                    itemSet.m_ConveyorPositions.push_back(t.matrix());
                }
                else
                {
                    Eigen::Affine3f t{Eigen::Translation3f(position2d.x(), headPosition.y() + 0.1f, position2d.y())};
                    itemSet.m_ConveyorPositions.push_back(t.matrix());
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

        auto translation = (position.m_Position).cast<float>();
        auto rotation = cpp_conv::rotationRadiansFromDirection(direction.m_Direction);
        Eigen::Affine3f t{Eigen::Translation3f(translation.x(), translation.y(), translation.z())};
        Eigen::Affine3f r{Eigen::AngleAxisf(rotation.AsRadians(), Eigen::Vector3f(0, 1, 0))};
        conveyors[type].m_ConveyorPositions.push_back((t * r).matrix());

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
                    Eigen::Affine3f t{Eigen::Translation3f(position2d.x(), position.m_Position.y() + 0.1f, position2d.y())};
                    itemSet.m_ConveyorPositions.push_back(t.matrix());
                }
                else
                {
                    Eigen::Affine3f t{Eigen::Translation3f(position2d.x(), position.m_Position.y() + 0.1f, position2d.y())};
                    itemSet.m_ConveyorPositions.push_back(t.matrix());
                }
            }
        }
    }

    const bool instancingSupported = 0 != (BGFX_CAPS_INSTANCING & bgfx::getCaps()->supported);
    for(auto& conveyorType : conveyors)
    {
        if (conveyorType.m_ConveyorPositions.empty())
        {
            continue;
        }

        if (instancingSupported)
        {
            atlas::render::drawInstanced(
                m_ViewId,
                conveyorType.m_Model,
                conveyorType.m_Model->GetProgram(),
                conveyorType.m_ConveyorPositions);
        }
        else
        {
            // We don't currently support non-instanced platforms
            assert(false);
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
            drawInstanced(
                m_ViewId,
                item.m_Model,
                item.m_Model->GetProgram(),
                item.m_ConveyorPositions,
                ~BGFX_DISCARD_STATE);
        }
        else
        {
            // We don't currently support non-instanced platforms
            assert(false);
        }
    }
}
