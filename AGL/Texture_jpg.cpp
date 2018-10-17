#include "Texture.h"
#include <rpp/debugging.h>

#if AGL_JPEG_SUPPORT
#  include <jpeglib.h>
#endif

namespace AGL
{
    ////////////////////////////////////////////////////////////////////////////////

#if AGL_JPEG_SUPPORT
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
            return Texture::createTexture(img, width, height, channels, false, true);
        }
    };
#endif // AGL_JPEG_SUPPORT

    ////////////////////////////////////////////////////////////////////////////////

    bool Bitmap::loadJPG(const void* imageData, int numBytes)
    {
        clear();
        #if AGL_JPEG_SUPPORT
            return JpegLoader{}.load(*this, imageData, numBytes);
        #else
            fprintf(stderr, "JPEG not supported in this build.");
            return false;
        #endif
    }

    ////////////////////////////////////////////////////////////////////////////////
}
