/**
 * Basic bitmap operations, Copyright (c) 2017-208, Jorma Rebane
 * Distributed under MIT Software License
 */
#pragma once
#include "AGLConfig.h"
#include <stdint.h>

#define AGL_BMP_SUPPORT 1
#define AGL_PNG_SUPPORT 1
#define AGL_JPEG_SUPPORT _WIN32

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
        uint8_t* Data = nullptr;
        int Width    = 0;
        int Height   = 0;
        int Channels = 0;
        int Stride   = 0;
        bool Owns = false; // Do we own this Data ptr? If yes, then ~Bitmap() calls free(Data)

        Bitmap();
        /**
         * Create a new Bitmap container for raw image data
         * @param freeDataWhenDone If true, then `data` will be freed in ~Bitmap()
         */
        Bitmap(uint8_t* data, int w, int h, int channels, int stride, bool freeDataWhenDone = false);
        ~Bitmap();
        Bitmap(Bitmap&& bitmap) noexcept;
        Bitmap& operator=(Bitmap&& bitmap) noexcept;

        explicit operator bool() const { return Data && Width && Height; }

        Bitmap(const Bitmap& bitmap) = delete;
        Bitmap& operator=(const Bitmap& bitmap) = delete;

        /**
         * Free all data
         */
        void clear();

        /**
         * Converts this image data from BGR <-> RGB
         */
        void bgr2rgb();

        /**
         * Performs a vertical flip on the image data.
         */
        void verticalFlip();

        static Bitmap create(unsigned glTexture);
        static Bitmap create(int width, int height, int channels, FromFrameBuffer);

        bool loadPNG(const void* imageData, int numBytes);
        bool loadJPG(const void* imageData, int numBytes);
        bool loadBMP(const void* imageData, int numBytes);
    };

    /**
     * Converts raw bitmap data from BGR <-> RGB
     */
    AGL_API void bgr2rgb(uint8_t* bitmapData, int w, int h, int channels, int stride);
}

