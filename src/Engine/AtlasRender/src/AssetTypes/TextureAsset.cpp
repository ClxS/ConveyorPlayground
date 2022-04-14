#include "AtlasRenderPCH.h"
#include "TextureAsset.h"

#include <iostream>

#include "AtlasCore/FileReader.h"
#include "AtlasResource/FileData.h"
#include "bimg/decode.h"
#include "bx/allocator.h"

namespace
{
    bx::AllocatorI* getDefaultAllocator()
    {
        static bx::DefaultAllocator s_allocator;
        return &s_allocator;
    }

    void imageReleaseCb(void* ptr, void* userData)
    {
        const auto imageContainer = static_cast<bimg::ImageContainer*>(userData);
        bimg::imageFree(imageContainer);
    }

    bgfx::TextureHandle loadTexture(
        const atlas::core::FileReader reader,
        const uint64_t flags = BGFX_TEXTURE_NONE|BGFX_SAMPLER_NONE,
        bgfx::TextureInfo* info = nullptr,
        bimg::Orientation::Enum* orientation = nullptr)
    {
        bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
        bimg::ImageContainer* imageContainer = bimg::imageParse(
            getDefaultAllocator(),
            reader.GetData(),
            static_cast<uint32_t>(reader.GetTotalSize()));

        if (imageContainer == nullptr)
        {
            std::cerr << "Failed to parse image";
            return {};
        }

        if (orientation != nullptr)
        {
            *orientation = imageContainer->m_orientation;
        }

        const bgfx::Memory* mem = bgfx::makeRef(
                  imageContainer->m_data
                , imageContainer->m_size
                , imageReleaseCb
                , imageContainer
                );

        if (imageContainer->m_cubeMap)
        {
            handle = bgfx::createTextureCube(
                  static_cast<uint16_t>(imageContainer->m_width)
                , 1 < imageContainer->m_numMips
                , imageContainer->m_numLayers
                , static_cast<bgfx::TextureFormat::Enum>(imageContainer->m_format)
                , flags
                , mem
                );
        }
        else if (1 < imageContainer->m_depth)
        {
            handle = bgfx::createTexture3D(
                  static_cast<uint16_t>(imageContainer->m_width)
                , static_cast<uint16_t>(imageContainer->m_height)
                , static_cast<uint16_t>(imageContainer->m_depth)
                , 1 < imageContainer->m_numMips
                , static_cast<bgfx::TextureFormat::Enum>(imageContainer->m_format)
                , flags
                , mem
                );
        }
        else if (bgfx::isTextureValid(0, false, imageContainer->m_numLayers, static_cast<bgfx::TextureFormat::Enum>(imageContainer->m_format), flags) )
        {
            handle = bgfx::createTexture2D(
                  static_cast<uint16_t>(imageContainer->m_width)
                , static_cast<uint16_t>(imageContainer->m_height)
                , 1 < imageContainer->m_numMips
                , imageContainer->m_numLayers
                , static_cast<bgfx::TextureFormat::Enum>(imageContainer->m_format)
                , flags
                , mem
                );
        }

        if (bgfx::isValid(handle))
        {
            // TODO Add HANDLE NAMING
            //bgfx::setName(handle, _filePath);
        }

        if (info != nullptr)
        {
            bgfx::calcTextureSize(
                  *info
                , static_cast<uint16_t>(imageContainer->m_width)
                , static_cast<uint16_t>(imageContainer->m_height)
                , static_cast<uint16_t>(imageContainer->m_depth)
                , imageContainer->m_cubeMap
                , 1 < imageContainer->m_numMips
                , imageContainer->m_numLayers
                , static_cast<bgfx::TextureFormat::Enum>(imageContainer->m_format)
                );
        }

        return handle;
    }
}

atlas::render::TextureAsset::TextureAsset(bgfx::TextureHandle textureHandle): m_Texture{textureHandle}
{
}

atlas::render::TextureAsset::~TextureAsset()
{
    destroy(m_Texture);
    m_Texture = {};
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> atlas::render::textureLoadHandler(
    const resource::FileData& data)
{
    const core::FileReader reader{ data.m_pData.get(), data.m_Size };
    const auto handle = loadTexture(reader);
    if (!isValid(handle))
    {
        return nullptr;
    }

    return std::make_shared<TextureAsset>(handle);
}
