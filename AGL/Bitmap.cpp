#include "Bitmap.h"
#include <memory>
#include "OpenGL.h"

namespace AGL
{
    ////////////////////////////////////////////////////////////////////////////////

    static int AlignRowTo4(int width, int channels) // glTexImage2d requires rows to be 4-byte aligned
    {
        int stride = width * channels;
        return stride + 3 - ((stride - 1) % 4);
    }

    ////////////////////////////////////////////////////////////////////////////////

    Bitmap::Bitmap() = default;
    Bitmap::Bitmap(uint8_t * data, int w, int h, int channels, int stride, bool freeDataWhenDone)
        : Data{data}, Width{w}, Height{h}, Channels{channels}, Stride{stride}, Owns{freeDataWhenDone}
    {
    }
    Bitmap::~Bitmap()
    {
        if (Owns) free(Data);
    }
    Bitmap::Bitmap(Bitmap&& bitmap) noexcept
    {
        this->operator=(std::move(bitmap));
    }
    Bitmap& Bitmap::operator=(Bitmap&& bitmap) noexcept
    {
        std::swap(Data,     bitmap.Data);
        std::swap(Width,    bitmap.Width);
        std::swap(Height,   bitmap.Height);
        std::swap(Channels, bitmap.Channels);
        std::swap(Stride,   bitmap.Stride);
        std::swap(Owns,     bitmap.Owns);
        return *this;
    }

    void Bitmap::clear()
    {
        if (Owns) free(Data);
        Data = nullptr;
        Width = Height = Channels = Stride = 0;
        Owns = false;
    }

    void Bitmap::bgr2rgb()
    {
        AGL::bgr2rgb(Data, Width, Height, Channels, Stride);
    }

    void Bitmap::verticalFlip()
    {
        int h = Height;
        int stride = Stride;
        uint8_t* src = Data;
        uint8_t* dst = Data + (h - 1)*stride;
        while (src < dst)
        {
            memcpy(dst, src, stride);
            src += stride;
            dst -= stride;
        }
    }

    Bitmap Bitmap::create(unsigned glTexture)
    {
        glFlushErrors();

        Bitmap bmp;

        glGetTextureLevelParameteriv(glTexture, 0, GL_TEXTURE_WIDTH, &bmp.Width);
        if (auto err = glGetErrorStr()) ThrowErr("Fatal: glGetTextureLevelParameteriv failed: %s", err);

        glGetTextureLevelParameteriv(glTexture, 0, GL_TEXTURE_HEIGHT, &bmp.Height);
        if (auto err = glGetErrorStr()) ThrowErr("Fatal: glGetTextureLevelParameteriv failed: %s", err);

        bmp.Channels = 3;
        bmp.Stride = AlignRowTo4(bmp.Width, bmp.Channels);
        bmp.Data = (uint8_t*)malloc(size_t(bmp.Stride) * bmp.Height);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp.Data);
        if (auto err = glGetErrorStr()) ThrowErr("Fatal: glGetTexImage failed: %s", err);

        return bmp;
    }

    Bitmap Bitmap::create(int width, int height, int channels, FromFrameBuffer)
    {
        glFlushErrors();

        Bitmap bmp;
        bmp.Width = width;
        bmp.Height = height;
        bmp.Channels = channels;
        bmp.Stride = AlignRowTo4(bmp.Width, channels);
        bmp.Data = (uint8_t*)malloc(size_t(bmp.Stride) * bmp.Height);

        unsigned type = GL_BGR;
        if (channels == 1) type = GL_RED;
        if (channels == 4) type = GL_BGRA;

        glReadPixels(0, 0, width, height, type, GL_UNSIGNED_BYTE, bmp.Data);
        if (auto err = glGetErrorStr())
            ThrowErr("Fatal: glReadPixels failed: %s", err);
        return bmp;
    }

    ////////////////////////////////////////////////////////////////////////////////

    void bgr2rgb(uint8_t* bitmapData, int w, int h, int channels, int stride)
    {
        if (channels == 3)
        {
            uint8_t* ptr = bitmapData;
            for (int y = 0; y < h; ++y)
            {
                uint8_t* row = &ptr[stride * y];
                for (int x = 0; x < w; ++x)
                {
                    uint8_t tmp = row[0];
                    row[0] = row[2];
                    row[2] = tmp;
                    row += 3;
                }
            }
        }
        else if (channels == 4)
        {
            uint8_t* ptr = bitmapData;
            for (int y = 0; y < h; ++y)
            {
                uint8_t* row = &ptr[stride * y];
                for (int x = 0; x < w; ++x)
                {
                    uint8_t tmp = row[0];
                    row[0] = row[2];
                    row[2] = tmp;
                    row += 4;
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
}
