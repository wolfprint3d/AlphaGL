/**
 * Basic bitmap operations, Copyright (c) 2017-208, Jorma Rebane
 * Distributed under MIT Software License
 */
#pragma once
#include "AGLConfig.h"
#include <stdint.h>

namespace AGL
{
    struct FromFrameBuffer {};

    /**
     * Simple bitmap data in RAM.
     * Can be used for transferring texture data to other API's
     */
    class AGL_API Bitmap
    {
    public:
        int Width    = 0;
        int Height   = 0;
        int Channels = 0;
        int Stride   = 0;
        uint8_t* Data = nullptr;

        Bitmap();
        ~Bitmap();
        Bitmap(Bitmap&& bitmap) noexcept;
        Bitmap& operator=(Bitmap&& bitmap) noexcept;

        Bitmap(const Bitmap& bitmap) = delete;
        Bitmap& operator=(const Bitmap& bitmap) = delete;

        static Bitmap create(unsigned glTexture);
        static Bitmap create(int width, int height, int channels, FromFrameBuffer);
    };
}

