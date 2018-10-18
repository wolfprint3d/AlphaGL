/**
 * OpenGL Texture operations, Copyright (c) 2017-208, Jorma Rebane
 * Distributed under MIT Software License
 */
#pragma once
#include <rpp/vec.h>
#include "AGLConfig.h"
#include "Bitmap.h"

namespace AGL
{
    using std::vector;
    using std::string;
    using rpp::strview;
    using rpp::uint;
    using rpp::Rect;
    using rpp::Vector2;
    ////////////////////////////////////////////////////////////////////////////////


    enum TextureHint 
    {
        TexHintNone,
        TexHintBMP,
        TexHintPNG,
        TexHintJPG,
    };


    static constexpr bool isPowerOfTwo(int n) // 64,128,256,512,...
    {
        return (n & (n - 1)) == 0;
    }


    /**
     * Resource wrapper for OpenGL texture handles
     * Textures are stored in GPU texture memory
     * For accessing the data, either use OpenGL texture data functions,
     * or use the Bitmap API via `Texture::getBitmap()`
     */
    class AGL_API Texture
    {
        string texname; // for debugging, mostly
        uint glTexture = 0; // OpenGL texture
        int glWidth    = 0;
        int glHeight   = 0;
        int glChannels = 0;
        bool glTiled   = false;
    public:

        // if set to TRUE, any texture loads will make use of Driver side texture compression
        // this means slower loads, but less GPU memory usage
        static bool GPUCompression;

        Texture() noexcept;
        explicit Texture(const char* filename);
        explicit Texture(const string& filename);
        Texture(const void* data, int size, TextureHint hint = TexHintNone);
        ~Texture();
        
        Texture(Texture&& t) noexcept; // Enable MOVE
        Texture& operator=(Texture&&) noexcept;
        Texture(const Texture&)            = delete;  // NOCOPY
        Texture& operator=(const Texture&) = delete;

        bool good()               const { return glTexture || glWidth; }
        explicit operator bool()  const { return glTexture || glWidth; }
        bool operator!()          const { return !glTexture && !glWidth; }
        
        /** @return TRUE if the texture is loading from a remote source */
        bool isLoading() const { return !glTexture && glWidth; }
        
        /** @return TRUE if the GL texture can be bound, this can be interpreted as "isLoaded" */
        bool isBindable() const { return glTexture != 0; }

        int width()  const { return glWidth; }
        int height() const { return glHeight; }
        const string& name() const { return texname; }
        Vector2 size() const { return Vector2{ (float)glWidth, (float)glHeight }; }
        uint nativeHandle() const { return glTexture; }

        /**
         * Loads an image from a file into GPU texture memory
         */
        bool loadFromFile(const string& filename);

        /**
         * Loads image data such as JPG, PNG, BMP
         * into raw texture data into GPU texture memory
         * This is used by loadFromFile()
         */
        bool loadBitmap(const void* bitmapData, int numBytes, TextureHint hint);

        /**
         * Loads raw data into GPU texture memory
         * @note each row must be aligned to 4-byte boundary
         */
        bool load(const void* data, int width, int height, int channels, int stride);
        bool load(const Bitmap& bmp);

        /**
         * Unload texture from GPU memory
         */
        void unload();

        // Enables tiling for this texture - only works if the texture is a power of two (GLES limitation)
        void enableTextureTiling(bool enable=true);

        void bind();
        void unbind();

        // Gets the padded size of the texture data
        int getTextureDataSize() const;

        // call getTextureDataSize() to get the minimum required bytes
        bool getTextureData(void* paddedDestination, bool bgr = false);
        std::vector<uint8_t> getTextureData(bool bgr = false);
        Bitmap getBitmap(bool bgr = false);

        /**
         * Creates a new OpenGL texture from raw image data
         * @param imageData Allocated image data.
         * @note each row must be aligned to 4-byte boundary
         * @param w
         * @param h
         * @param channels
         * @return Texture handle on success, 0 on failure
         */
        static uint createTexture(void* imageData, int w, int h, int channels);

        // Saves this texture as a BMP file, @warning: texture will be rebound
        bool saveAsBMP(strview fileName);
        static bool saveAsBMP(strview fileName, const void* data, int width, int height, int channels);
    };

    ////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Weak reference to a texture with a rect that provides 
     *        a sub-rect into the texture.
     */
    struct AGL_API TextureRef
    {
        /** @brief WEAK REF to a texture */
        const Texture* texture;
        
        /** @brief Sub-rect giving a view inside the texture */
        Rect rect;

        TextureRef();
        TextureRef(const Texture* tex);
        TextureRef(const Texture& tex);
        TextureRef(const Texture& tex, Rect rect);

        operator bool() const { return texture  && texture->good(); }
        bool operator!()const { return !texture || !texture->good(); }

        bool operator==(const TextureRef& t) const {
            return texture == t.texture && rect == t.rect;
        }
        bool operator!=(const TextureRef& t) const {
            return texture != t.texture || rect != t.rect;
        }
        
        bool isLoading()  const { return texture && texture->isLoading(); }
        bool isBindable() const { return texture && texture->isBindable(); }

        const Texture* operator->() const { return texture; }
        Texture* operator->() { return const_cast<Texture*>(texture); }

        const Vector2& size() const { return rect.size; }

        /**
         * @brief Transforms the sub-rect view into texture coordinates
         */
        void getCoordinates(float& outLeft, float& outTop, float& outRight, float& outBottom) const;
    };

    ////////////////////////////////////////////////////////////////////////////////

    // glTexImage2d requires rows to be 4-byte aligned,
    // which is what this function helps to achieve
    int AlignRowTo4(int width, int channels);

    // Pads all rows to 4-byte alignment and returns the complete padded size
    int PaddedImageSize(int width, int height, int channels);

    ////////////////////////////////////////////////////////////////////////////////
}

