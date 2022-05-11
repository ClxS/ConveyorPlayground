#include "AtlasRenderPCH.h"
#include "AssetTypes/ShaderAsset.h"

#include <iostream>
#include <string>

#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceLoader.h"

namespace
{
    struct ProgramHeader
    {
        struct ProgramChunk
        {
            uint32_t m_Start;
            uint32_t m_End;
        };

        struct TextureSlotInfo
        {
            uint8_t m_Slot;
            uint8_t m_IsReserved;
            ProgramChunk m_Type;
        };

        ProgramChunk m_VertexShader;
        ProgramChunk m_FragmentShader;
        int32_t m_TextureSlotCount;
        int32_t m_TextureSlotInfoCount;
        ProgramChunk m_TextureSlotInfo;
    };
}

atlas::render::ShaderAsset::ShaderAsset(bgfx::ShaderHandle handle): m_Handle{handle}
{}

atlas::render::ShaderAsset::~ShaderAsset()
{
    bgfx::destroy(m_Handle);
}

atlas::render::VertexShader::VertexShader(const bgfx::ShaderHandle handle)
    : ShaderAsset{handle}
{
}

atlas::render::FragmentShader::FragmentShader(const bgfx::ShaderHandle handle)
    : ShaderAsset{handle}
{
}

atlas::render::ShaderProgram::ShaderProgram(
    bgfx::ProgramHandle handle,
    resource::AssetPtr<VertexShader> vertex,
    resource::AssetPtr<FragmentShader> fragment,
    const int64_t textureSlotCount,
    std::vector<SlotInfo> textureSlotInformation)
    : m_ProgramHandle{handle}
    , m_Vertex{std::move(vertex)}
    , m_Fragment{std::move(fragment)}
    , m_TextureSlotInformation{std::move(textureSlotInformation)}
    , m_TextureSlotCount{textureSlotCount}
{

}

atlas::render::ShaderProgram::~ShaderProgram()
{
    bgfx::destroy(m_ProgramHandle);
    m_Vertex.reset();
    m_Fragment.reset();
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> atlas::render::vertexShaderLoadHandler(
    const resource::FileData& data)
{
    const bgfx::Memory* mem = bgfx::copy(data.m_pData.get(), static_cast<uint32_t>(data.m_Size));
    const bgfx::ShaderHandle handle = bgfx::createShader(mem);
    setName(handle, "vertexShader");

    return std::make_shared<VertexShader>(handle);
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> atlas::render::fragmentShaderLoadHandler(
    const atlas::resource::FileData& data)
{
    const bgfx::Memory* mem = bgfx::copy(data.m_pData.get(), static_cast<uint32_t>(data.m_Size));
    const bgfx::ShaderHandle handle = bgfx::createShader(mem);
    setName(handle, "fragmentShader");

    return std::make_shared<FragmentShader>(handle);
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> atlas::render::shaderProgramLoadHandler(
    const resource::FileData& data)
{
    const auto dataAsChar = reinterpret_cast<const char*>(data.m_pData.get());
    const auto program = reinterpret_cast<ProgramHeader*>(data.m_pData.get());
    const std::string_view vertexName {
        &dataAsChar[program->m_VertexShader.m_Start],
        program->m_VertexShader.m_End - program->m_VertexShader.m_Start + 1};
    const std::string_view fragmentName {
        &dataAsChar[program->m_FragmentShader.m_Start],
        program->m_FragmentShader.m_End - program->m_FragmentShader.m_Start + 1};

    const auto vertexBundleId = resource::ResourceLoader::LookupId(vertexName);
    if (!vertexBundleId.has_value() || !vertexBundleId->IsValid())
    {
        std::cerr << std::format("Could not find resource with name {}\n", vertexName);
        return nullptr;
    }

    const auto fragmentBundleId = resource::ResourceLoader::LookupId(fragmentName);
    if (!fragmentBundleId.has_value() || !fragmentBundleId->IsValid())
    {
        std::cerr << std::format("Could not find resource with name {}\n", fragmentName);
        return nullptr;
    }

    const auto vertexShader = resource::ResourceLoader::LoadAsset<VertexShader>(vertexBundleId.value());
    if (!vertexShader)
    {
        std::cerr << std::format("Failed to load resource with name {}. Bundle: {}, Id: {}\n", vertexName, vertexBundleId.value().m_BundleIndex, vertexBundleId.value().m_BundleId.m_Value);
        return nullptr;
    }

    const auto fragmentShader = resource::ResourceLoader::LoadAsset<FragmentShader>(fragmentBundleId.value());
    if (!fragmentShader)
    {
        std::cerr << std::format("Failed to load resource with name {}. Bundle: {}, Id: {}\n", fragmentName, fragmentBundleId.value().m_BundleIndex, fragmentBundleId.value().m_BundleId.m_Value);
        return nullptr;
    }

    std::vector<ShaderProgram::SlotInfo> textureSlotInformation;
    if (program->m_TextureSlotInfo.m_Start != 0 && program->m_TextureSlotInfo.m_End != 0)
    {
        const auto slotInfos = reinterpret_cast<const ProgramHeader::TextureSlotInfo*>(&data.m_pData[program->m_TextureSlotInfo.m_Start]);
        for(int i = 0; i < program->m_TextureSlotInfoCount; i++)
        {
            constexpr size_t stride = sizeof(ProgramHeader::TextureSlotInfo);
            const auto info = slotInfos + stride * i;

            std::string_view type;
            if (info->m_Type.m_Start != 0)
            {
                type = {
                    &dataAsChar[info->m_Type.m_Start],
                    info->m_Type.m_End - info->m_Type.m_Start + 1};
            }

            textureSlotInformation.emplace_back(info->m_Slot, info->m_IsReserved > 0, core::hashing::fnv1(type));
        }
    }

    auto programHandle = bgfx::createProgram(vertexShader->GetHandle(), fragmentShader->GetHandle(), false);
    return std::make_shared<ShaderProgram>(programHandle, vertexShader, fragmentShader, program->m_TextureSlotCount, textureSlotInformation);
}
