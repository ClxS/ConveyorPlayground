#include "UIControllerSystem.h"

#include <cstdint>
#include <RmlUi/Core.h>
#include <SDL_timer.h>

#include "AssetRegistry.h"
#include "Constants.h"
#include "AtlasAppHost/Application.h"
#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasRender/AssetTypes/TextureAsset.h"
#include "AtlasResource/ResourceAsset.h"
#include "AtlasResource/ResourceLoader.h"
#include "bgfx/bgfx.h"
#include "Eigen/Core"
#include "Eigen/Geometry"
#include "RmlUi/Debugger/Debugger.h"

class RmlRawDataAsset : public atlas::resource::ResourceAsset
{
public:
    explicit RmlRawDataAsset(std::filesystem::path pagePath, std::unique_ptr<uint8_t[]> data, size_t dataSize)
        : m_PagePath{std::move(pagePath)}
        , m_Data{std::move(data)}
        , m_DataSize{dataSize}
    {
    }

    [[nodiscard]] const std::filesystem::path& GetPath() const { return m_PagePath; }
    [[nodiscard]] const uint8_t* GetData() const { return m_Data.get(); }
    [[nodiscard]] size_t GetDataSize() const { return m_DataSize; }

protected:
    std::filesystem::path m_PagePath;
    std::unique_ptr<uint8_t[]> m_Data;
    size_t m_DataSize;
};

class RmlUIPage final : public atlas::resource::ResourceAsset
{
public:
    explicit RmlUIPage(std::filesystem::path pagePath, std::string data)
        : m_PagePath{std::move(pagePath)}
    , m_Data{std::move(data)}
    {

    }

    [[nodiscard]] const std::filesystem::path& GetPath() const { return m_PagePath; }
    [[nodiscard]] const std::string& GetString() const { return m_Data; }

protected:
    std::filesystem::path m_PagePath;
    std::string m_Data;
};

class RmlUICss final : public atlas::resource::ResourceAsset
{
public:
    explicit RmlUICss(std::filesystem::path pagePath, std::string data)
        : m_PagePath{std::move(pagePath)}
        , m_Data{std::move(data)}
    {

    }

    [[nodiscard]] const std::filesystem::path& GetPath() const { return m_PagePath; }
    [[nodiscard]] const std::string& GetString() const { return m_Data; }

protected:
    std::filesystem::path m_PagePath;
    std::string m_Data;
};

class RmlUIFont final : public RmlRawDataAsset
{
public:
    explicit RmlUIFont(std::filesystem::path pagePath, std::unique_ptr<uint8_t[]> data, const size_t dataSize)
        : RmlRawDataAsset(std::move(pagePath), std::move(data), dataSize)
    {}
};

class RmlRenderInterface final : public Rml::RenderInterface
{
public:
    RmlRenderInterface()
    {
        m_RmlVertexLayout
        .begin()
        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

        m_TexturedProgram = atlas::resource::ResourceLoader::LoadAsset<
            cpp_conv::resources::registry::CoreBundle,
        atlas::render::ShaderProgram>(cpp_conv::resources::registry::core_bundle::shaders::ui::c_rmlui_basic_textured);
        m_UntexturedProgram = atlas::resource::ResourceLoader::LoadAsset<
            cpp_conv::resources::registry::CoreBundle,
            atlas::render::ShaderProgram>(cpp_conv::resources::registry::core_bundle::shaders::ui::c_rmlui_basic_untextured);
        m_Uniforms.m_Transform = createUniform("u_transform", bgfx::UniformType::Mat3);
        m_Uniforms.m_Sampler = createUniform("s_texColor", bgfx::UniformType::Sampler);
    }

    Rml::CompiledGeometryHandle CompileGeometry(Rml::Vertex* vertices, const int numVertices, int* indices, const int numIndices, Rml::TextureHandle texture) override
    {
        const bgfx::Memory* vertexMemory = bgfx::alloc(m_RmlVertexLayout.getSize(numVertices));
        const bgfx::Memory* indexMemory = bgfx::alloc(numIndices * sizeof(int));

        std::memcpy(vertexMemory->data, vertices, m_RmlVertexLayout.getSize(numVertices));
        std::memcpy(indexMemory->data, indices, numIndices * sizeof(int));

        const auto vertexBuffer = bgfx::createVertexBuffer(vertexMemory, m_RmlVertexLayout);
        const auto indexBuffer = bgfx::createIndexBuffer(indexMemory, BGFX_BUFFER_INDEX32);

        const auto handle = ++m_CompiledGeometryCounter;
        m_CompiledGeometry[handle] =
        {
            vertexBuffer,
            indexBuffer,
            texture
        };

        return handle;
    }

    void RenderCompiledGeometry(const Rml::CompiledGeometryHandle geometryHandle, const Rml::Vector2f& translation) override
    {
        const auto geometryIter = m_CompiledGeometry.find(geometryHandle);
        if (geometryIter == m_CompiledGeometry.end())
        {
            return;
        }

        const auto& geometry = *geometryIter;

        SetTransformUniform(translation);
        bgfx::setVertexBuffer(0, geometry.second.m_VertexBuffer);
        bgfx::setIndexBuffer(geometry.second.m_IndexBuffer);
        bgfx::setState(0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
            );

        if (geometry.second.m_Texture)
        {
            const auto textureIter = m_Textures.find(geometry.second.m_Texture);
            if (textureIter == m_Textures.end())
            {
                bgfx::submit(cpp_conv::constants::render_views::c_ui, m_UntexturedProgram->GetHandle());
            }
            else
            {
                bgfx::setTexture(0, m_Uniforms.m_Sampler, textureIter->second.m_Handle);
                bgfx::submit(cpp_conv::constants::render_views::c_ui, m_TexturedProgram->GetHandle());
            }
        }
        else
        {
            bgfx::submit(cpp_conv::constants::render_views::c_ui, m_UntexturedProgram->GetHandle());
        }
    }

    void ReleaseCompiledGeometry(const Rml::CompiledGeometryHandle geometryHandle) override
    {
        const auto geometryIter = m_CompiledGeometry.find(geometryHandle);
        if (geometryIter == m_CompiledGeometry.end())
        {
            return;
        }

        const auto& geometry = *geometryIter;
        destroy(geometry.second.m_VertexBuffer);
        destroy(geometry.second.m_IndexBuffer);
        m_CompiledGeometry.erase(geometryIter);
    }

    void RenderGeometry(
        Rml::Vertex* vertices,
        const int numVertices,
        int* indices,
        const int numIndices,
        const Rml::TextureHandle texture,
        const Rml::Vector2f& translation) override
    {
        if (!m_TexturedProgram ||
            getAvailTransientVertexBuffer(numVertices, m_RmlVertexLayout) != static_cast<uint32_t>(numVertices) ||
            bgfx::getAvailTransientIndexBuffer(numIndices, true) != static_cast<uint32_t>(numIndices))
        {
            return;
        }

        bgfx::TransientVertexBuffer vb{};
        bgfx::TransientIndexBuffer ib{};

        allocTransientVertexBuffer(&vb, numVertices, m_RmlVertexLayout);
        allocTransientIndexBuffer(&ib, numIndices, true);

        std::memcpy(vb.data, vertices, m_RmlVertexLayout.getSize(numVertices));
        std::memcpy(ib.data, indices, numIndices * sizeof(int));

        SetTransformUniform(translation);
        setVertexBuffer(0, &vb);
        setIndexBuffer(&ib);
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        if (texture)
        {
            const auto textureIter = m_Textures.find(texture);
            if (textureIter == m_Textures.end())
            {
                bgfx::submit(cpp_conv::constants::render_views::c_ui, m_UntexturedProgram->GetHandle());
            }
            else
            {
                bgfx::setTexture(0, m_Uniforms.m_Sampler, textureIter->second.m_Handle);
                bgfx::submit(cpp_conv::constants::render_views::c_ui, m_TexturedProgram->GetHandle());
            }
        }
        else
        {
            bgfx::submit(cpp_conv::constants::render_views::c_ui, m_UntexturedProgram->GetHandle());
        }
    }

    void SetScissorRegion(const int x, const int y, const int width, const int height) override
    {
        m_ScissorRegion = { x, y, width, height };
    }

    bool LoadTexture(Rml::TextureHandle& textureHandle, Rml::Vector2i& textureDimensions, const Rml::String& source) override
    {
        const auto id = atlas::resource::ResourceLoader::LookupId(source);
        if (!id.has_value())
        {
            return false;
        }

        const auto texture = atlas::resource::ResourceLoader::LoadAsset<atlas::render::TextureAsset>(id.value());
        if (!texture)
        {
            return false;
        }

        const auto handle = ++m_TextureCounter;
        m_Textures[handle] =
        {
            texture,
            texture->GetHandle(),
            texture->GetWidth(),
            texture->GetHeight()
        };

        textureDimensions = { static_cast<int>(texture->GetWidth()), static_cast<int>(texture->GetHeight()) };
        textureHandle = handle;
        return handle;
    }

    bool GenerateTexture(Rml::TextureHandle& textureHandle, const Rml::byte* source, const Rml::Vector2i& sourceDimensions) override
    {
        constexpr uint32_t c_stride = 4;
        const uint32_t size = c_stride * sourceDimensions.x * sourceDimensions.y;

        if (!bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::RGBA8, 0) )
        {
            return false;
        }

        const bgfx::Memory* mem = bgfx::alloc(size);
        std::memcpy(mem->data, source, mem->size);

        const auto texHandle = bgfx::createTexture2D(
                  static_cast<uint16_t>(sourceDimensions.x),
                  static_cast<uint16_t>(sourceDimensions.y),
                  false,
                  1,
                  bgfx::TextureFormat::RGBA8,
                  0,
                  mem);

        if (!bgfx::isValid(texHandle))
        {
            return false;
        }

        const auto handle = ++m_TextureCounter;
        m_Textures[handle] =
        {
            nullptr,
            texHandle,
            static_cast<uint32_t>(sourceDimensions.x),
            static_cast<uint32_t>(sourceDimensions.y)
        };

        textureHandle = handle;
        return handle;
    }

    void ReleaseTexture(const Rml::TextureHandle texture) override
    {
        const auto textureIter = m_Textures.find(texture);
        if (textureIter == m_Textures.end())
        {
            return;
        }

        if (!textureIter->second.m_Asset)
        {
            destroy(textureIter->second.m_Handle);
        }

        m_Textures.erase(textureIter);
    }

    void EnableScissorRegion(const bool enable) override
    {
        if (enable)
        {
            bgfx::setViewScissor(cpp_conv::constants::render_views::c_ui,
                static_cast<uint16_t>(m_ScissorRegion[0]),
                static_cast<uint16_t>(m_ScissorRegion[1]),
                static_cast<uint16_t>(m_ScissorRegion[2]),
                static_cast<uint16_t>(m_ScissorRegion[3]));
        }
        else
        {
            bgfx::setViewScissor(cpp_conv::constants::render_views::c_ui, 0, 0, 0, 0);
        }
    }

private:
    void SetTransformUniform(const Rml::Vector2f& translation) const
    {
        auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();
        Eigen::Vector4f frameBufferSize = { static_cast<float>(width), static_cast<float>(height), 0.0f, 0.0f };

        const Eigen::Affine2f t
                { Eigen::Translation2f(
                    translation[0] - frameBufferSize.x() / 2.0f,
                    translation[1] - frameBufferSize.y() / 2.0f) };
        const Eigen::Affine2f s { Eigen::Scaling((1.0f / frameBufferSize.x()) * 2.0f, (1.0f / frameBufferSize[1]) * 2.0f) };
        Eigen::Affine2f inversionScaling;
        if (!bgfx::getCaps()->originBottomLeft)
        {
            // RmlUI structures with the assumption that bottom left is 0,0. We need to manually flip if it is not
            inversionScaling = Eigen::Scaling(1.0f, -1.0f);
        }
        else
        {
            inversionScaling = Eigen::Scaling(1.0f, 1.0f);
        }

        Eigen::Matrix3f m = (inversionScaling * s * t).matrix();
        bgfx::setUniform(m_Uniforms.m_Transform, m.data());
    }

    struct CompiledGeometry
    {
        bgfx::VertexBufferHandle m_VertexBuffer{BGFX_INVALID_HANDLE};
        bgfx::IndexBufferHandle m_IndexBuffer{BGFX_INVALID_HANDLE};
        Rml::TextureHandle m_Texture{};
    };

    struct Texture
    {
        atlas::resource::AssetPtr<atlas::render::TextureAsset> m_Asset{};
        bgfx::TextureHandle m_Handle{};
        uint32_t m_Width{};
        uint32_t m_Height{};
    };

    struct
    {
        bgfx::UniformHandle m_Transform{BGFX_INVALID_HANDLE};
        bgfx::UniformHandle m_Sampler{BGFX_INVALID_HANDLE};
    } m_Uniforms;

    Eigen::Vector4i m_ScissorRegion;
    atlas::resource::AssetPtr<atlas::render::ShaderProgram> m_TexturedProgram;
    atlas::resource::AssetPtr<atlas::render::ShaderProgram> m_UntexturedProgram;
    bgfx::VertexLayout m_RmlVertexLayout{};

    Rml::CompiledGeometryHandle m_CompiledGeometryCounter{};
    std::unordered_map<Rml::CompiledGeometryHandle, CompiledGeometry> m_CompiledGeometry;

    Rml::TextureHandle m_TextureCounter{};
    std::unordered_map<Rml::TextureHandle, Texture> m_Textures;
};

class RmlSystemInterface final : public Rml::SystemInterface
{
public:
    // RmlUi requests the current time and provides various utilities through this interface.
    /* ... */
    double GetElapsedTime() override
    {
	    return static_cast<double>(SDL_GetTicks()) / 1000.0;
	}
};

class RmlFileInterface final : public Rml::FileInterface
{
public:
    Rml::FileHandle Open(const Rml::String& path) override
    {
        if (path == "rml.rcss")
        {
            const auto asset = atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::registry::CoreBundle, RmlRawDataAsset>(
                        cpp_conv::resources::registry::core_bundle::ui::rcss::c_rml);
            if (!asset)
            {
                return {};
            }

            const auto handle = ++m_IncrementingHandle;
            m_OpenAssets[handle] =
            {
                asset
            };

            return handle;
        }

        return {};
    }

    void Close(const Rml::FileHandle file) override
    {
        const auto fileIt = m_OpenAssets.find(file);
        if (fileIt != m_OpenAssets.end())
        {
            m_OpenAssets.erase(fileIt);
        }
    }
    size_t Read(void* buffer, const size_t size, const Rml::FileHandle file)  override
    {
        const auto fileIter = m_OpenAssets.find(file);
        if (fileIter == m_OpenAssets.end())
        {
            assert(false);
            return 0;
        }

        auto& fileRef = fileIter->second;

        const auto remainingSpace = fileRef.m_File->GetDataSize() - fileRef.m_CurrentPosition;
        const auto toRead = std::min(remainingSpace, size);

        std::memcpy(buffer, fileRef.m_File->GetData(), toRead);

        fileRef.m_CurrentPosition += toRead;
        return toRead;
    }
    bool Seek(const Rml::FileHandle file, const long offset, const int origin)  override
    {
        const auto fileIter = m_OpenAssets.find(file);
        if (fileIter == m_OpenAssets.end())
        {
            assert(false);
            return false;
        }

        const auto size = fileIter->second.m_File->GetDataSize();

        //origin One of either SEEK_SET (seek from the beginning of the file), SEEK_END (seek from the end of the file) or SEEK_CUR (seek from the current file position).
        int64_t targetPosition = 0;
        switch (origin)
        {
            case SEEK_SET:
                targetPosition = offset;
                break;
            case SEEK_END:
                targetPosition = static_cast<int64_t>(size) - offset;
                break;
            case SEEK_CUR:
                targetPosition = static_cast<int64_t>(fileIter->second.m_CurrentPosition) + offset;
                break;
            default:
                assert(false);
                break;
        }

        if (targetPosition < 0 || targetPosition > static_cast<int64_t>(size))
        {
            return false;
        }

        fileIter->second.m_CurrentPosition = targetPosition;
        return true;
    }
    size_t Tell(const Rml::FileHandle file)  override
    {
        return m_OpenAssets[file].m_CurrentPosition;
    }

private:
    Rml::FileHandle m_IncrementingHandle{};

    struct DataReference
    {
        atlas::resource::AssetPtr<RmlRawDataAsset> m_File;
        size_t m_CurrentPosition{};
    };

    std::unordered_map<Rml::FileHandle, DataReference> m_OpenAssets;

    static_assert(sizeof(size_t) <= sizeof(Rml::FileHandle));
};

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> rmlUiPageLoader(const atlas::resource::FileData& data)
{
    std::string page = reinterpret_cast<const char*>(data.m_pData.get());
    return std::make_shared<RmlUIPage>(data.m_FilePath, page);
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> rmlUiCssLoader(const atlas::resource::FileData& data)
{
    std::string page = reinterpret_cast<const char*>(data.m_pData.get());
    return std::make_shared<RmlUICss>(data.m_FilePath, page);
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> rmlUiFontLoader(atlas::resource::FileData& data)
{
    return std::make_shared<RmlUIFont>(data.m_FilePath, std::move(data.m_pData), data.m_Size);
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> rawRmlDataLoader(atlas::resource::FileData& data)
{
    return std::make_shared<RmlRawDataAsset>(data.m_FilePath, std::move(data.m_pData), data.m_Size);
}

cpp_conv::UIControllerSystem::UIControllerSystem()
{
}

cpp_conv::UIControllerSystem::~UIControllerSystem()
{
}

void cpp_conv::UIControllerSystem::Initialise(atlas::scene::EcsManager& ecsManager)
{
    atlas::resource::ResourceLoader::RegisterTypeHandler<RmlRawDataAsset>(rawRmlDataLoader);

    atlas::resource::ResourceLoader::RegisterTypeHandler<RmlUIPage>(rmlUiPageLoader);
    atlas::resource::ResourceLoader::RegisterTypeHandler<RmlUICss>(rmlUiCssLoader);
    atlas::resource::ResourceLoader::RegisterTypeHandler<RmlUIFont>(rmlUiFontLoader);

    m_FileInterface = std::make_unique<RmlFileInterface>();
    m_SystemInterface = std::make_unique<RmlSystemInterface>();
    m_RenderInterface = std::make_unique<RmlRenderInterface>();

    Rml::SetFileInterface(m_FileInterface.get());
    Rml::SetSystemInterface(m_SystemInterface.get());
    Rml::SetRenderInterface(m_RenderInterface.get());

    Rml::Initialise();

    Rml::Debugger::SetVisible(true);

    auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();

    // Create a context to display documents within.
    m_Context = std::unique_ptr<Rml::Context, std::function<void(Rml::Context*)>>(
        Rml::CreateContext("main", Rml::Vector2i(width, height)),
        [](Rml::Context* ctx) { Rml::RemoveContext("main"); });

    const auto page = atlas::resource::ResourceLoader::LoadAsset<resources::registry::CoreBundle, RmlUIPage>(
        resources::registry::core_bundle::ui::pages::c_helloWorld);
    const auto mainFont = atlas::resource::ResourceLoader::LoadAsset<resources::registry::CoreBundle, RmlUIFont>(
        resources::registry::core_bundle::ui::fonts::c_LatoLatin_Regular);
    const auto backupFont = atlas::resource::ResourceLoader::LoadAsset<resources::registry::CoreBundle, RmlUIFont>(
        resources::registry::core_bundle::ui::fonts::c_NotoEmoji_Regular);

    if (!page || !mainFont || !backupFont)
    {
        return;
    }

    m_Pages.push_back(page);

    assert(Rml::LoadFontFace(
        mainFont->GetData(),
        static_cast<int>(mainFont->GetDataSize()),
        "LatoLatin",
        Rml::Style::FontStyle::Normal,
        Rml::Style::FontWeight::Normal));

    assert(Rml::LoadFontFace(
        backupFont->GetData(),
        static_cast<int>(backupFont->GetDataSize()),
        "Noto",
        Rml::Style::FontStyle::Normal,
        Rml::Style::FontWeight::Normal));

    // Now we are ready to load our document.
    Rml::ElementDocument* document = m_Context->LoadDocumentFromMemory(page->GetString());
    document->Show();
}

void cpp_conv::UIControllerSystem::Update(atlas::scene::EcsManager&)
{
    if (!m_Context)
    {
        return;
    }

    // Update the context to reflect any changes resulting from input events, animations,
    // modified and added elements, or changed data in data bindings.
    m_Context->Update();

    // Render the user interface. All geometry and other rendering commands are now
    // submitted through the render interface.
    m_Context->Render();
}
