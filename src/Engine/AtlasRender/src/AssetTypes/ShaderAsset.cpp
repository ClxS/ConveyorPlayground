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

        ProgramChunk m_VertexShader;
        ProgramChunk m_FragmentShader;
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
        std::cerr << std::format("Could not find resource with name {}", vertexName);
        return nullptr;
    }

    const auto fragmentBundleId = resource::ResourceLoader::LookupId(fragmentName);
    if (!fragmentBundleId.has_value() || !fragmentBundleId->IsValid())
    {
        std::cerr << std::format("Could not find resource with name {}", fragmentName);
        return nullptr;
    }

    const auto vertexShader = resource::ResourceLoader::LoadAsset<VertexShader>(vertexBundleId.value());
    if (!vertexShader)
    {
        std::cerr << std::format("Failed to load resource with name {}. Bundle: {}, Id: {}", vertexName, vertexBundleId.value().m_BundleIndex, vertexBundleId.value().m_BundleId.m_Value);
        return nullptr;
    }

    const auto fragmentShader = resource::ResourceLoader::LoadAsset<FragmentShader>(fragmentBundleId.value());
    if (!fragmentShader)
    {
        std::cerr << std::format("Failed to load resource with name {}. Bundle: {}, Id: {}", fragmentName, fragmentBundleId.value().m_BundleIndex, fragmentBundleId.value().m_BundleId.m_Value);
        return nullptr;
    }

    auto programHandle = bgfx::createProgram(vertexShader->GetHandle(), fragmentShader->GetHandle(), false);
    return std::make_shared<ShaderProgram>(programHandle, vertexShader, fragmentShader);
}
