#include "AtlasRenderPCH.h"

#include "ModelAsset.h"
#include "Renderer.h"

void atlas::render::draw(
    const bgfx::ViewId viewId,
    const resource::AssetPtr<ModelAsset>& model,
    const resource::AssetPtr<ShaderProgram>& program,
    const Eigen::Matrix4f& transform)
{
    for(const auto& segment : model->GetMesh()->GetSegments())
    {
        bgfx::setTransform(transform.data());

        setVertexBuffer(0, segment.m_VertexBuffer);
        setIndexBuffer(segment.m_IndexBuffer);

        uint8_t textureIndex = 0;
        for(const auto& texture : model->GetTextures())
        {
            setTexture(textureIndex++, texture.m_Sampler, texture.m_Texture->GetHandle());
        }

        submit(viewId, program->GetHandle());
    }
}

void atlas::render::drawInstanced(
    const bgfx::ViewId viewId,
    const resource::AssetPtr<ModelAsset>& model,
    const resource::AssetPtr<ShaderProgram>& program,
    const std::vector<Eigen::Matrix4f>& transforms)
{
    constexpr uint16_t instanceStride = sizeof(Eigen::Matrix4f);

    const auto totalPositions = static_cast<uint32_t>(transforms.size());
    const uint32_t numDrawableInstances = bgfx::getAvailInstanceDataBuffer(totalPositions, instanceStride);

    bgfx::InstanceDataBuffer idb{};
    bgfx::allocInstanceDataBuffer(&idb, numDrawableInstances, instanceStride);

    // TODO Log how many entities couldn't be drawn
    for (uint32_t i = 0; i < numDrawableInstances; ++i)
    {
        const uint32_t offset = i * instanceStride;
        std::memcpy(&(idb.data[offset]), transforms[i].data(), instanceStride);
    }

    uint8_t textureIndex = 0;
    for(const auto& texture : model->GetTextures())
    {
        setTexture(textureIndex++, texture.m_Sampler, texture.m_Texture->GetHandle());
    }

    for(const auto& segment : model->GetMesh()->GetSegments())
    {
        setVertexBuffer(0, segment.m_VertexBuffer);
        setIndexBuffer(segment.m_IndexBuffer);
        setInstanceDataBuffer(&idb);
        submit(viewId, program->GetHandle());
    }
}
