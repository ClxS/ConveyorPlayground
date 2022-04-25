#include "ModelRenderSystem.h"

#include "Constants.h"
#include "DirectionComponent.h"
#include "ModelComponent.h"
#include "PositionComponent.h"
#include "Transform2D.h"
#include "AtlasRender/AssetTypes/ModelAsset.h"

void cpp_conv::ModelRenderSystem::Update(atlas::scene::EcsManager& ecs)
{
    using namespace components;
    for(auto [entity, model, position] : ecs.IterateEntityComponents<ModelComponent, PositionComponent>())
    {
        if (!model.m_Model || !model.m_Model->GetMesh() || !model.m_Model->GetProgram())
        {
            continue;
        }

        assert(0 != (BGFX_CAPS_INSTANCING & bgfx::getCaps()->supported));

        atlas::maths_helpers::Angle rotation{};
        if (ecs.DoesEntityHaveComponent<DirectionComponent>(entity))
        {
            const auto& direction = ecs.GetComponent<DirectionComponent>(entity);
            rotation = rotationRadiansFromDirection(direction.m_Direction);
        }

        constexpr uint16_t instanceStride = sizeof(Eigen::Matrix4f);

        constexpr uint32_t totalPositions = 1;
        const uint32_t numDrawableInstances = bgfx::getAvailInstanceDataBuffer(totalPositions, instanceStride);

        bgfx::InstanceDataBuffer idb{};
        allocInstanceDataBuffer(&idb, numDrawableInstances, instanceStride);

        // Only recompute these on-dirty, once the dirtying system is added
        Eigen::Affine3f t{Eigen::Translation3f(position.m_Position.cast<float>())};
        Eigen::Affine3f r{Eigen::AngleAxisf(rotation.AsRadians(), Eigen::Vector3f(0, 1, 0))};
        Eigen::Matrix4f m = (t * r).matrix();
        bgfx::setTransform(m.data());
        for(const auto& segment : model.m_Model->GetMesh()->GetSegments())
        {
            std::memcpy(idb.data, m.data(), instanceStride);
            setInstanceDataBuffer(&idb);
            setVertexBuffer(0, segment.m_VertexBuffer);
            setIndexBuffer(segment.m_IndexBuffer);

            int textureIndex = 0;
            for(const auto& texture : model.m_Model->GetTextures())
            {
                setTexture(textureIndex++, texture.m_Sampler, texture.m_Texture->GetHandle());
            }

            submit(cpp_conv::constants::render_views::c_geometry, model.m_Model->GetProgram()->GetHandle());
        }
    }
}
