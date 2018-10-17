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
    Bitmap::~Bitmap()
    {
        if (Data) free(Data);
    }
    Bitmap::Bitmap(Bitmap&& bitmap) noexcept
    {
        this->operator=(std::move(bitmap));
    }
    Bitmap& Bitmap::operator=(Bitmap&& bitmap) noexcept
    {
        std::swap(Width,    bitmap.Width);
        std::swap(Height,   bitmap.Height);
        std::swap(Channels, bitmap.Channels);
        std::swap(Data,     bitmap.Data);
        return *this;
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
}
