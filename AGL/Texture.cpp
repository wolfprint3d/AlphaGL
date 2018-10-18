#include "Texture.h"
#include "OpenGL.h"
#include <rpp/file_io.h>

namespace AGL
{
    using std::swap;
    using std::move;
    using namespace rpp::literals;
    using rpp::ushort;
    using rpp::ulong;
    ////////////////////////////////////////////////////////////////////////////////

    bool Texture::GPUCompression = false;

    ////////////////////////////////////////////////////////////////////////////////
    
    Texture::Texture() noexcept = default;

    Texture::Texture(const char* filename)
    {
        loadFromFile(filename);
    }
    Texture::Texture(const string& filename)
    {
        loadFromFile(filename);
    }
    Texture::Texture(const void* data, int size, TextureHint hint)
    {
        loadBitmap(data, size, hint);
    }
    Texture::~Texture()
    {
        if (glTexture) {
            glDeleteTextures(1, &glTexture);
        }
    }

    Texture::Texture(Texture&& t) noexcept
    {
        this->operator=(move(t));
    }

    Texture& Texture::operator=(Texture&& t) noexcept
    {
        swap(texname,    t.texname);
        swap(glTexture,  t.glTexture);
        swap(glWidth,    t.glWidth);
        swap(glHeight,   t.glHeight);
        swap(glChannels, t.glChannels);
        swap(glTiled,    t.glTiled);
        return *this;
    }

    static TextureHint getTextureHint(const string& filename)
    {
        strview ext = rpp::file_ext(filename);
        if      (ext.equalsi("png"_sv))  return TexHintPNG; // We mostly use PNG
        else if (ext.equalsi("jpg"_sv)
              || ext.equalsi("jpeg"_sv)) return TexHintJPG;
        else if (ext.equalsi("bmp"_sv))  return TexHintBMP;
        return TexHintNone;
    }

    bool Texture::loadFromFile(const string& filename)
    {
        if (glTexture) {
            LogWarning("warning: tried to load already loaded texture with '%s'", filename.c_str());
            return true; // we already have a texture; success.
        }

        texname = filename;
        if (auto buf = rpp::file::read_all(filename)) {
            return loadBitmap(buf.data(), buf.size(), getTextureHint(filename));
        }

        LogWarning("failed to load file '%s'", filename.c_str());
        return false;
    }

    bool Texture::loadBitmap(const void* bitmapData, int numBytes, TextureHint hint)
    {
        if (glTexture) {
            LogWarning("warning: duplicate texture load '%s'", texname.c_str());
            return true; // we already have a texture; success.
        }

        Bitmap bitmap;
        switch (hint) {
            case TexHintPNG: bitmap.loadPNG(bitmapData, numBytes); break;
            case TexHintJPG: bitmap.loadJPG(bitmapData, numBytes); break;
            case TexHintBMP: bitmap.loadBMP(bitmapData, numBytes); break;
            default:         LogError("error: unsupported image format: %d", hint);
        }
        return load(bitmap);
    }

    bool Texture::load(const void* data, int width, int height, int channels, int stride)
    {
        if (!data || width <= 0 || height <= 0 || channels <= 0) {
            LogError("invalid texture data: %p %dx%dpx ch:%d str:%d", data, width, height, channels, stride);
            return false;
        }
        Bitmap bitmap { (uint8_t*)data, width, height, channels, stride, false };
        return load(bitmap);
    }

    bool Texture::load(const Bitmap& bmp)
    {
        glTexture  = createTexture((void*)bmp.Data, bmp.Width, bmp.Height, bmp.Channels);
        glWidth    = bmp.Width;
        glHeight   = bmp.Height;
        glChannels = bmp.Channels;
        if (!glTexture) {
            LogError("failed to generate GL texture");
        }
        return glTexture != 0;
    }

    void Texture::unload()
    {
        if (glTexture) {
            glDeleteTextures(1, &glTexture);
            glTexture = 0, glWidth = 0, glHeight = 0, glChannels = 0;
            glTiled = false;
        }
    }

    void Texture::enableTextureTiling(bool enable)
    {
        if (enable && (!isPowerOfTwo(glWidth) || !isPowerOfTwo(glHeight))) {
            LogError("Enabling GLES texture tiling requires power of two textures");
        }
        
        glTiled = enable;
        if (glTexture)
        {
            glBindTexture(GL_TEXTURE_2D, glTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, enable ? GL_REPEAT : GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, enable ? GL_REPEAT : GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void Texture::bind()
    {
        glBindTexture(GL_TEXTURE_2D, glTexture);
    }

    void Texture::unbind()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    int Texture::getTextureDataSize() const
    {
        return PaddedImageSize(glWidth, glHeight, glChannels);
    }

    bool Texture::getTextureData(void* paddedDestination, bool bgr)
    {
        if (!glTexture) {
            LogError("No texture data! '%s'", texname.c_str());
            return false;
        }

        glFlushErrors();
        bind();
        {
            constexpr GLenum bgrFormats[] = { GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_BGR, GL_BGRA };
            constexpr GLenum rgbFormats[] = { GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };
            GLenum format = bgr ? bgrFormats[glChannels - 1] : rgbFormats[glChannels - 1];
            glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, paddedDestination);
        }
        unbind();

        if (auto err = glGetErrorStr()) {
            LogError("Fatal: glGetTexImage '%s' failed: %s", texname.c_str(), err);
            return false;
        }
        return true;
    }

    vector<uint8_t> Texture::getTextureData(bool bgr)
    {
        vector<uint8_t> image; image.resize(size_t(getTextureDataSize()));
        getTextureData(image.data(), bgr);
        return image;
    }


    Bitmap Texture::getBitmap(bool bgr)
    {
        int stride = AlignRowTo4(glWidth, glChannels);
        Bitmap bmp {
            (uint8_t*)malloc(size_t(glHeight) * stride),
            glWidth,
            glHeight,
            glChannels,
            stride,
            true
        };
        getTextureData(bmp.Data, bgr);
        return bmp;
    }


    ////////////////////////////////////////////////////////////////////////////////

    uint Texture::createTexture(void* imageData, int w, int h, int channels)
    {
        GLenum imgFmt = GL_LUMINANCE;
        if      (channels == 2) imgFmt = GL_LUMINANCE_ALPHA;
        else if (channels == 3) imgFmt = GL_RGB;
        else if (channels == 4) imgFmt = GL_RGBA;

        GLenum gpuFmt = imgFmt;
    #ifndef __EMSCRIPTEN__
        if (Texture::GPUCompression)
        {
            constexpr GLenum compressedFormats[] = {
                #if !__IPHONEOS__
                    GL_COMPRESSED_LUMINANCE,
                    GL_COMPRESSED_LUMINANCE_ALPHA,
                    GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT };
                #else
                    GL_LUMINANCE,
                    GL_LUMINANCE_ALPHA,
                    GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG,
                    GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG };
                #endif
            gpuFmt = compressedFormats[channels - 1];
        }
    #endif

        glFlushErrors();

        uint glTexture; glGenTextures(1, &glTexture);
        if (!glTexture) {
            LogError("error: glGenTexture failed. Did you bind a valid GL context?");
        }

        glBindTexture(GL_TEXTURE_2D, glTexture);

        // OpenGLES mipmapping is limited
        const bool pow2 = isPowerOfTwo(w) && isPowerOfTwo(h);
        // @todo This causes rendering bug on Windows, so wtf?
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pow2 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

        // GLES on iOS requires this to enable NPOT textures:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // 4 is the default value
        glTexImage2D(GL_TEXTURE_2D, 0, gpuFmt, w, h, 0, imgFmt, GL_UNSIGNED_BYTE, imageData);

        if (const char* err = glGetErrorStr()) {
            LogError("glTexImage2D failed: %s", err);
            glDeleteTextures(1, &glTexture);
            return 0;
        }

        if (pow2) glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps

        glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture
        return glTexture;
    }


    ////////////////////////////////////////////////////////////////////////////////


    TextureRef::TextureRef() : texture(nullptr), rect(Rect::Zero())
    {
    }

    TextureRef::TextureRef(const Texture* tex) 
        : texture(tex), rect(Vector2::Zero(), tex ? tex->size() : Vector2::Zero())
    {
    }

    TextureRef::TextureRef(const Texture& tex) : texture(&tex), rect(Vector2::Zero(),tex.size())
    {
    }

    TextureRef::TextureRef(const Texture& tex, Rect rect) : texture(&tex), rect(rect)
    {
    }
    
    void TextureRef::getCoordinates(float& outLeft, float& outTop, float& outRight, float& outBottom) const
    {
        Vector2 texSize = texture->size();
        outLeft   = rect.x / texSize.x;
        outTop    = 1.0f - ((rect.y + rect.h) / texSize.y);
        outRight  = (rect.x + rect.w) / texSize.x;
        outBottom = 1.0f - (rect.y / texSize.y);
    }


    ////////////////////////////////////////////////////////////////////////////////


    int AlignRowTo4(int width, int channels) // glTexImage2d requires rows to be 4-byte aligned
    {
        int stride = width * channels;
        return stride + 3 - ((stride - 1) % 4);
    }

    int PaddedImageSize(int width, int height, int channels)
    {
        return AlignRowTo4(width, channels) * height;
    }

    ////////////////////////////////////////////////////////////////////////////////
}
