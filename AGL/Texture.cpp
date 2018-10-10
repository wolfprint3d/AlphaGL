#include "Texture.h"
#include "OpenGL.h"
#include <rpp/file_io.h>

#include <png.h>
#include <jpeglib.h>

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
    
    Texture::Texture() noexcept
    {
    }
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
        loadFromData(data, size, hint);
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
        if (auto buf = rpp::file::read_all(filename))
            return loadFromData(buf.data(), buf.size(), getTextureHint(filename));

        LogWarning("failed to load file '%s'", filename.c_str());
        return false;
    }

    bool Texture::loadFromData(const void* data, int size, TextureHint hint)
    {
        if (glTexture) {
            LogWarning("warning: duplicate texture load '%s'", texname.c_str());
            return true; // we already have a texture; success.
        }

        switch (hint) {
            case TexHintPNG: glTexture = loadPNG(data, size, glWidth, glHeight, glChannels); break;
            case TexHintJPG: glTexture = loadJPG(data, size, glWidth, glHeight, glChannels); break;
            case TexHintBMP: glTexture = loadBMP(data, size, glWidth, glHeight, glChannels); break;
            default:         LogError("error: unsupported image format: %d", hint);
        }

        if (!glTexture) {
            LogError("failed to generate GL texture: '%s'", texname.c_str());
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

    void Texture::enableTextureTiling(const bool enable)
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

    ////////////////////////////////////////////////////////////////////////////////

    TextureRef::TextureRef() : texture(0), rect(Rect::Zero())
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

    uint Texture::createTexture(void* allocatedImage, int w, int h, int channels)
    {
        constexpr GLenum uncompressedFormats[] = { GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };
        GLenum imgFmt = uncompressedFormats[channels - 1];
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pow2 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

        // GLES on iOS requires this to enable NPOT textures:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // this is the default value
        glTexImage2D(GL_TEXTURE_2D, 0, gpuFmt, w, h, 0, imgFmt, GL_UNSIGNED_BYTE, allocatedImage);
        free(allocatedImage);

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

    class PngLoader
    {
        static void Err(png_structp, const char* err) {
            LogError("png error: %s", err);
        }
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, &Err, 0);
        png_infop  info = png_create_info_struct(png);
    public:
        ~PngLoader() { png_destroy_read_struct(&png, &info, 0); }
        struct png_io_data {
            const char* ptr;
            const char* end;
        };
        static void PngMemReader(png_structp png, png_bytep dstbuf, size_t numBytes) {
            png_io_data* io = (png_io_data*)png_get_io_ptr(png);
            size_t avail = io->end - io->ptr;
            if (numBytes > avail)
                numBytes = avail;
            memcpy(dstbuf, io->ptr, numBytes);
            io->ptr += numBytes;
        }
        uint load(const void* data, int size, int& outWidth, int& outHeight, int& outChannels)
        {
            if (size <= 8 || !png_check_sig((png_bytep)data, 8)) {
                LogError("png error: invalid png signature");
                return 0;
            }
            while (const char* err = glGetErrorStr()) {
                LogWarning("Errors before loadPNG: %s  "
                           "Make sure you are loading textures on main thread!", err);
            }

            png_io_data io { (char*)data + 8, (char*)data + size };
            png_set_read_fn(png, &io, &PngMemReader);
            png_set_sig_bytes(png, 8);
            png_read_info(png, info);

            uint32_t width  = 0;
            uint32_t height = 0;
            int bitDepth    = 0;
            int colorType   = -1;
            uint32_t ret = png_get_IHDR(png, info, &width, &height, &bitDepth, &colorType, 0, 0, 0);
            if (ret != 1) {
                LogError("png error: failed to read PNG header");
                return 0;
            }

            // http://www.libpng.org/pub/png/libpng-1.2.5-manual.html#section-3.7
            if (bitDepth == 16) // strip 16-bit PNG to 8-bit
                png_set_strip_16(png);
            if (colorType == PNG_COLOR_TYPE_PALETTE)
                png_set_palette_to_rgb(png);
            else if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
                png_set_expand_gray_1_2_4_to_8(png);
            if (png_get_valid(png, info, (png_uint_32)PNG_INFO_tRNS))
                png_set_tRNS_to_alpha(png);

            // now update info based on new unpacking and expansion flags:
            png_read_update_info(png, info);
            bitDepth  = png_get_bit_depth(png, info);
            colorType = png_get_color_type(png, info);

            //printf("png %dx%d bits:%d type:%s\n", width, height, bitDepth, strPNGColorType(colorType));

            int channels;
            switch (colorType) {
                default:case PNG_COLOR_TYPE_GRAY:channels = 1; break;
                case PNG_COLOR_TYPE_GRAY_ALPHA:  channels = 2; break;
                case PNG_COLOR_TYPE_PALETTE:
                case PNG_COLOR_TYPE_RGB:         channels = 3; break;
                case PNG_COLOR_TYPE_RGB_ALPHA:   channels = 4; break;
            }

            // Do a double query on what libpng says the rowbytes are and what
            // we are assuming. If our rowBytes is wrong, then colorType switch 
            // has a bug and GL format is wrong. It will most likely segfault.
            int stride = AlignRowTo4(width, channels);
            Assert((width*channels) == (int)png_get_rowbytes(png, info), "pngRowBytes is invalid");

            uint8_t* img = (uint8_t*)malloc(stride * height);
            if (!img) { // most likely corrupted image
                LogError("Failed to allocate %d bytes", stride * height);
                return 0;
            }

            outWidth    = width;
            outHeight   = height;
            outChannels = channels;

            // OpenGL eats images in reverse row order, so read all rows in reverse
            for (int y = height-1; y >= 0; --y)
            {
                uint8_t* row = img + y * stride;
                png_read_row(png, (png_bytep)row, nullptr);
            }
            return Texture::createTexture(img, width, height, channels);
        }
    };


    ////////////////////////////////////////////////////////////////////////////////
    ///////// LIBJPEG
    class JpegLoader
    {
        jpeg_decompress_struct cinfo = { nullptr };
        jpeg_error_mgr jerr = { nullptr };
    public:
        ~JpegLoader()
        {
            jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);
        }
        static void Err(j_common_ptr cinfo) {
            char errorMessage[JMSG_LENGTH_MAX];
            cinfo->err->format_message(cinfo, errorMessage);
            LogError("jpg load failed: %s", errorMessage);
        };
        uint load(const void* data, int size, int& outWidth, int& outHeight, int& outChannels)
        {
            jerr.error_exit = &Err;
            cinfo.err = jpeg_std_error(&jerr);

            jpeg_create_decompress(&cinfo);
            jpeg_mem_src(&cinfo, (png_bytep)data, (ulong)size);
            if (jpeg_read_header(&cinfo, TRUE/*require image*/) != JPEG_HEADER_OK) {
                LogWarning("Invalid JPG header");
                return 0;
            }

            jpeg_start_decompress(&cinfo);
            int width    = cinfo.output_width;
            int height   = cinfo.output_height;
            int channels = cinfo.output_components;
            int stride   = AlignRowTo4(width, channels);

            uint8_t* img = (uint8_t*)malloc(size_t(stride * height));
            if (!img) { // most likely corrupted image
                LogError("Failed to allocate %d bytes", stride * height);
                return 0;
            }

            JSAMPARRAY buf = cinfo.mem->alloc_sarray((j_common_ptr)&cinfo, JPOOL_IMAGE, uint(stride), 1);
            for (int y = 0; y < height; ++y)
            {
                assert(y == cinfo.output_scanline);
                jpeg_read_scanlines(&cinfo, buf, 1);

                // OpenGL eats images in reverse row order, so write all rows in reverse
                int reverseY = (height - 1) - y;
                assert(0 <= reverseY && reverseY < height);
                uint8_t* row = &img[reverseY * stride];
                memcpy(row, buf[0], size_t(stride));
            }

            outWidth    = width;
            outHeight   = height;
            outChannels = channels;
            return Texture::createTexture(img, width, height, channels);
        }
    };


    ////////////////////////////////////////////////////////////////////////////////
    //// ---- BMP format structures ---- ////
    #pragma pack(push)
    #pragma pack(1) // make sure no struct alignment packing is made
    struct CIEXYZTRIPLE { struct CIEXYZ { long x, y, z; } r, g, b; };
    struct RGBQUAD { uint8_t b, g, r, x; };
    struct BitmapFileHeader { ushort Type; uint Size, Reserved, OffBits; };
    struct BitmapInfoHeader { uint Size; int Width, Height; ushort Planes, BitCount; uint Compression, SizeImage; uint XPelsPerMeter, YPelsPerMeter; uint ClrUsed, ClrImportant; };
    struct BitmapV5InfoHeader { BitmapInfoHeader BIH; uint RedMask, GreenMask, BlueMask, AlphaMask, CSType; CIEXYZTRIPLE EndPoints; uint GammaRed, GammaGreen, GammaBlue, Intent, ProfileData, ProfileSize, Reserved; };
    #pragma pack(pop)

    static bool savePaddedBMP(strview fileName, const void* paddedData, int width, int height, int channels)
    {
        rpp::file file = { fileName, rpp::CREATENEW };
        if (!file) {
            LogError("Failed to create BMP file '%s'", fileName.to_cstr());
            return false;
        }

        int paddedSize = AlignRowTo4(width, channels) * height;

        // 8-bit data requires a color table (!)
        if (channels == 1)
        {
            RGBQUAD colors[256]; // we need to create a color table for an 8-bit image:
            const auto setColorTableBGRX = [](RGBQUAD (&col)[256], int i) {
                uint8_t v = uint8_t(i);
                col[i].b = v, col[i].g = v, col[i].r = v, col[i].x = 0;
            };
            for(int i = 0; i < 256; i += 4) { // unrolled for win
                setColorTableBGRX(colors, i+0);
                setColorTableBGRX(colors, i+1);
                setColorTableBGRX(colors, i+2);
                setColorTableBGRX(colors, i+3);
            }
            constexpr uint HSize = sizeof(BitmapFileHeader) + sizeof(BitmapV5InfoHeader) + sizeof(colors);
            BitmapFileHeader bmfV5 = { 19778, HSize + paddedSize, 0, HSize,	};
            BitmapV5InfoHeader bmiV5 = { {
                    sizeof(BitmapV5InfoHeader), // size of the V5 struct
                    width, height,  // height of the image - negative for top-down bitmap
                    ushort(1), ushort(8u), 0u,	// 1 plane, 8bpp, BI_RGB(uncompressed)
                    uint(paddedSize),			// image data size
                    3780u, 3780u,				// X/YPelsPerMeter
                    256u, 0u					// 256 colors in the color table, all colors
                }, 0u, // set rest to 0
            };
            bmiV5.CSType = 0x01; // device dependent RGB colorspace
            bmiV5.Intent = 8;    // LCS_GM_ABS_COLORIMETRIC - use nearest palette match
            file.write(&bmfV5,  sizeof(bmfV5));
            file.write(&bmiV5,  sizeof(bmiV5));
            file.write(&colors, sizeof(colors));
        }
        else
        {
            BitmapFileHeader bmf = { 19778u, uint(54 + paddedSize), 0, 54, };
            BitmapInfoHeader bmi = {
                sizeof(BitmapInfoHeader),	// size of this struct
                width, height,					 // height of the image - negative for top-down bitmap
                ushort(1), ushort(channels*8),0u,// 1 plane, number of bits: 8/24/32, BI_RGB(uncompressed)
                uint(paddedSize),				 // size of image
                3780u, 3780u,					 // X/YPelsPerMeter
                0u, 0u,							 // No colortable
            };
            file.write(&bmf, sizeof(bmf));
            file.write(&bmi, sizeof(bmi));
        }
        file.write(paddedData, paddedSize);
        return true;
    }

    bool Texture::saveAsBMP(strview fileName, const void* data, int width, int height, int channels)
    {
        uint8_t* paddedData = nullptr;
        int unpaddedStride = channels * width;
        if ((unpaddedStride & 3) != 0) // do we need to align the rows? (BMP requirement)
        {
            int paddedStride = AlignRowTo4(width, channels);
            int paddedSize = paddedStride * height;
            paddedData = (uint8_t*)malloc(size_t(paddedSize));

            uint8_t* dst = paddedData;
            const uint8_t* src = (const uint8_t*)data;
            for (int i = 0; i < height; i++) // for each row in the image
            {
                // just copy the padded amount straight away
                // the padded data will be somewhat garbage, since it's taken from the next row
                memcpy(dst, src, size_t(paddedStride));
                dst += paddedStride; // advance dst by padded rows
                src += unpaddedStride; // advance src by raw data size
            }
            data = paddedData; // just overwrite the original data pointer (since we already made a copy)
        }

        bool success = savePaddedBMP(fileName, data, width, height, channels);
        if (paddedData) free(paddedData); // clean up if we used paddedData
        return success;
    }


    struct binary_reader {
        uint8_t* buf, *ptr;
        binary_reader(const void* data, int size) : buf((uint8_t*)data), ptr(buf) {}
        template<class T> T* read() { T* result = (T*)ptr; ptr += sizeof(T); return result; }
        void set(int offset) { ptr = buf + offset; }
        void read(uint8_t* dst, int size) const { memcpy(dst, ptr, size_t(size)); }
    };
    uint Texture::loadBMP(const void* data, int size, int& outWidth, int& outHeight, int& outChannels)
    {
        binary_reader reader = { data, size };
        auto& bmh = *reader.read<BitmapFileHeader>();
        if (bmh.Type != 19778) { // read BitmapFileHeader for OffBits
            return 0;
        }

        auto& bmi = *reader.read<BitmapInfoHeader>(); // read BitmapInfoHeader for size info
        int nchannels = bmi.BitCount >> 3;
        if (!(1 <= nchannels && nchannels <= 4)) {
            LogError("Corrupted BMP, invalid number of channels: %d", nchannels);
            return 0;
        }

        uint8_t* img = (uint8_t*)malloc(bmi.SizeImage); // allocate enough for the entire image
        reader.set(bmh.OffBits);  // seek to start of image data
        reader.read(img, bmi.SizeImage);
        outWidth    = bmi.Width;
        outHeight   = bmi.Height;
        outChannels = nchannels;
        return Texture::createTexture(img, bmi.Width, bmi.Height, nchannels);
    }

    ////////////////////////////////////////////////////////////////////////////////

    uint Texture::loadPNG(const void* data, int size, int& outWidth, int& outHeight, int& outChannels)
    {
        return PngLoader{}.load(data, size, outWidth, outHeight, outChannels);
    }

    uint Texture::loadJPG(const void* data, int size, int& outWidth, int& outHeight, int& outChannels)
    {
        return JpegLoader{}.load(data, size, outWidth, outHeight, outChannels);
    }

    bool Texture::saveAsBMP(strview fileName)
    {
        if (!glTexture)
            return false;

        constexpr bool useBGR = true;
        vector<uint8_t> image = this->getTextureData(useBGR);
        return savePaddedBMP(fileName, image.data(), glWidth, glHeight, glChannels);
    }

    ////////////////////////////////////////////////////////////////////////////////
}
