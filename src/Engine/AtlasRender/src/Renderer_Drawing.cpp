#include "AtlasRenderPCH.h"

#include "ModelAsset.h"
#include "Renderer.h"
#include "AtlasCore/Hashing.h"

namespace
{
    constexpr int c_maxShadows = 1;
    static std::vector<atlas::render::ShadowCaster> g_shadowMaps;
    static std::array<bgfx::UniformHandle, c_maxShadows> g_shadowMapSampler = BGFX_INVALID_HANDLE;
    static std::array<bgfx::UniformHandle, c_maxShadows> g_lightMtx = BGFX_INVALID_HANDLE;

    atlas::render::ShadowCaster getShadowCaster(const uint8_t shadowIndex)
    {
        if (g_shadowMaps.size() >= shadowIndex)
        {
            return g_shadowMaps[shadowIndex];
        }

        return { {}, BGFX_INVALID_HANDLE };
    }

    void setShadowState(const uint8_t textureSlot, const uint8_t shadowSlot)
    {
        assert(shadowSlot < c_maxShadows);

        const auto& caster = getShadowCaster(shadowSlot);
        setTexture(
            textureSlot,
            g_shadowMapSampler[shadowSlot],
            caster.m_ShadowMap,
            UINT32_MAX);
        setUniform(g_lightMtx[shadowSlot], caster.m_LightMatrix.data());
    }

    void setIntrinsicTextureSlots(const std::shared_ptr<atlas::render::ShaderProgram>& program)
    {
        for(const auto& slot : program->GetTextureSlotInfos())
        {
            switch (slot.m_TypeHash)
            {
            case 0: break;
            case atlas::core::hashing::fnv1("shadowMap"):
                setShadowState(slot.m_Slot, 0);
                break;
            default:
                assert(false); // "Unknown texture slot type"
                break;
            }
        }
    }
}

void atlas::render::setShadowCaster(const uint8_t shadowIndex, ShadowCaster shadow)
{
    g_shadowMaps.resize(shadowIndex + 1);
    g_shadowMaps[shadowIndex] = std::move(shadow);
}

void initDrawingData()
{
    for(int i = 0; i < c_maxShadows; ++i)
    {
        assert(i < 1); // TODO Add support for extended maps
        g_shadowMapSampler[i] = bgfx::createUniform("s_shadowMap", bgfx::UniformType::Sampler);
        g_lightMtx[i] = bgfx::createUniform("u_lightMtx", bgfx::UniformType::Mat4);
    }
}


void atlas::render::draw(
    const bgfx::ViewId viewId,
    const bgfx::VertexBufferHandle vertexBuffer,
    const resource::AssetPtr<ShaderProgram>& program,
    const Eigen::Matrix4f& transform,
    const uint8_t flags)
{
    setIntrinsicTextureSlots(program);

    bgfx::setTransform(transform.data());
    bgfx::setVertexBuffer(0, vertexBuffer);
    submit(viewId, program->GetHandle(), flags);
}

void atlas::render::draw(
    const bgfx::ViewId viewId,
    const bgfx::VertexBufferHandle vertexBuffer,
    const bgfx::IndexBufferHandle indexBuffer,
    const resource::AssetPtr<ShaderProgram>& program,
    const Eigen::Matrix4f& transform,
    const uint8_t flags)
{
    setIntrinsicTextureSlots(program);

    bgfx::setTransform(transform.data());
    bgfx::setVertexBuffer(0, vertexBuffer);
    bgfx::setIndexBuffer(indexBuffer);
    submit(viewId, program->GetHandle(), flags);
}

void atlas::render::draw(
    const bgfx::ViewId viewId,
    const resource::AssetPtr<ModelAsset>& model,
    const resource::AssetPtr<ShaderProgram>& program,
    const Eigen::Matrix4f& transform,
    const uint8_t flags)
{
    for(const auto& segment : model->GetMesh()->GetSegments())
    {
        setIntrinsicTextureSlots(program);

        bgfx::setTransform(transform.data());
        setVertexBuffer(0, segment.m_VertexBuffer);
        setIndexBuffer(segment.m_IndexBuffer);

        uint8_t textureIndex = 0;
        for(const auto& texture : model->GetTextures())
        {
            const auto textureInfo = program->GetTextureSlotInfo(textureIndex);
            if (texture.m_Texture && !textureInfo.m_bReserved)
            {
                setTexture(textureIndex++, texture.m_Sampler, texture.m_Texture->GetHandle());
            }
        }

        submit(viewId, program->GetHandle(), flags);
    }
}

void atlas::render::drawInstanced(
    const bgfx::ViewId viewId,
    const resource::AssetPtr<ModelAsset>& model,
    const resource::AssetPtr<ShaderProgram>& program,
    const std::vector<Eigen::Matrix4f>& transforms,
    const uint8_t flags)
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

    for(const auto& segment : model->GetMesh()->GetSegments())
    {
        setIntrinsicTextureSlots(program);

        uint8_t textureIndex = 0;
        for(const auto& texture : model->GetTextures())
        {
            const auto textureInfo = program->GetTextureSlotInfo(textureIndex);
            if (texture.m_Texture && !textureInfo.m_bReserved)
            {
                setTexture(textureIndex++, texture.m_Sampler, texture.m_Texture->GetHandle());
            }
        }

        setVertexBuffer(0, segment.m_VertexBuffer);
        setIndexBuffer(segment.m_IndexBuffer);
        setInstanceDataBuffer(&idb);
        submit(viewId, program->GetHandle(), flags);
    }
}
