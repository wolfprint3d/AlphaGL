//
// Created by Jorma on 28/05/17.
//
#pragma once
#include "AGLConfig.h"

namespace AGL
{
    ////////////////////////////////////////////////////////////////////////////////

    class AGL_API FrameBuffer
    {
        unsigned FrameBuf  = 0;
        unsigned RenderBuf = 0;
        unsigned Texture   = 0;
        int Width         = 0;
        int Height        = 0;
        int BytesPerPixel = 0;
        unsigned Format   = 0;

        // framebuffer that was active before this->bind() was called
        // during unbind() this will be set as the active framebuffer
        unsigned PushedFrameBuf = 0;

        // If true, this is a default framebuffer provided by the OpenGL context
        // Which means it won't be deleted
        bool DefaultFrameBuffer = false;

    public:

        FrameBuffer();
        ~FrameBuffer();

        void destroy();
        void create(int width, int height, unsigned format);
        void update(int width, int height);

        void bind();
        void unbind();

        // This will attempt to grab active context's default frame buffer
        // Not all contexts have a default frame buffer.
        // @return TRUE if default context was initialized
        bool initFromDefaultBuffer();

        bool good()         const { return FrameBuf != 0; }
        int width()         const { return Width; }
        int height()        const { return Height; }
        int bytesPerPixel() const { return BytesPerPixel; }

        // OpenGL framebuffer format type, such as GL_RGB
        unsigned format()   const { return Format; }

        // FrameBuffer texture handle
        unsigned texture() const { return Texture; }

        bool isDefaultBuffer() const { return DefaultFrameBuffer; }
    };

    struct BindFrameBuffer
    {
        FrameBuffer& FrameBuf;
        explicit BindFrameBuffer(FrameBuffer& frameBuf) : FrameBuf(frameBuf) { frameBuf.bind(); }
        ~BindFrameBuffer() { FrameBuf.unbind(); }
    };

    ////////////////////////////////////////////////////////////////////////////////
}

