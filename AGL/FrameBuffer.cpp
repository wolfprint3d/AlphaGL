#include "FrameBuffer.h"
#include "OpenGL.h"
#include <rpp/debugging.h>

namespace AGL
{
    ////////////////////////////////////////////////////////////////////////////////

    FrameBuffer::FrameBuffer() = default;

    FrameBuffer::~FrameBuffer()
    {
        destroy();
    }

    void FrameBuffer::destroy()
    {
        if (!DefaultFrameBuffer && FrameBuf)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteRenderbuffers(1, &RenderBuf), RenderBuf = 0;
            glDeleteTextures(1, &Texture),        Texture   = 0;
            glDeleteFramebuffers(1, &FrameBuf),   FrameBuf  = 0;
            Width = Height = BytesPerPixel = Format = 0;
        }
    }

    void FrameBuffer::create(int width, int height, unsigned format)
    {
        Format = format;
        update(width, height);
    }

    bool FrameBuffer::initFromDefaultBuffer()
    {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&FrameBuf);
        if (FrameBuf)
        {
            DefaultFrameBuffer = true;
            glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint*)&RenderBuf);

            glGetFramebufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &Width);
            glGetFramebufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &Height);
            glGetFramebufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, (GLint*)&Format);

            if (Format == GL_RGB)
            {
                BytesPerPixel = 3;
            }
            else
            {
                LogWarning("Unhandled GL framebuffer format: %d", Format);
            }

            LogInfo("Using framebuffer from GL context:  %dx%d  %d channels", Width, Height, BytesPerPixel);
            return true;
        }
        return false;
    }

    void FrameBuffer::update(int width, int height)
    {
        if (DefaultFrameBuffer)
            return;
        if (Width == width && Height == height)
            return;

        Width = width, Height = height;
        BytesPerPixel = 3;
        LogInfo("Updating framebuffer  %dx%d  %d channels", Width, Height, BytesPerPixel);

        if (!Texture) glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, Format, GL_UNSIGNED_BYTE, NULL);
        // GLES requires these to enable NPOT textures:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        if (!RenderBuf) glGenRenderbuffers(1, &RenderBuf);
        glBindRenderbuffer(GL_RENDERBUFFER, RenderBuf);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        if (!FrameBuf) glGenFramebuffers(1, &FrameBuf);
        glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderBuf);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        else
        {
            LogError("warning: failed to create framebuffer (%s)", glGetErrorStr());
            destroy();
        }
    }

    void FrameBuffer::bind()
    {
        if (PushedFrameBuf)
            LogError("warning: FrameBuffer was rebound before unbinding. Do you have nested framebuffer bindings?");

        unsigned currentBuf;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentBuf);
        if (currentBuf != FrameBuf)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, FrameBuf);
            PushedFrameBuf = currentBuf;
        }
    }

    void FrameBuffer::unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, PushedFrameBuf);
        PushedFrameBuf = 0;
    }

    ////////////////////////////////////////////////////////////////////////////////
}
