//
// Created by Jorma on 25/05/17.
//
#pragma once
#if __linux__
#include "OpenGL.h"
#include <GL/glx.h> // linux specific opengl X11

namespace AGL
{
    using std::string;
    ////////////////////////////////////////////////////////////////////////////////

    class AGL_API GLContext
    {
        // X11 stuff
        Display*   XDisplay  = nullptr;
        GLXContext XGLctx    = nullptr;
        Window     XWindow   = 0;
        Colormap   XColorMap = 0;
        GLXPbuffer XPBuffer  = 0;

        int Width         = 0;
        int Height        = 0;
        int BytesPerPixel = 0;

    public:

        GLContext();
        ~GLContext();

        int width()         const { return Width; }
        int height()        const { return Height; }
        int bytesPerPixel() const { return BytesPerPixel; }

        /**
         * Initialize OpenGL Context. Exception will be thrown on fatal errors.
         * @param width Desired width of the window and its back buffer
         * @param height Desired height of the window and its back buffer
         * @param createWindow If true create a window, else create a pixel buffer target
         * @param openglVersion [in] Desired GL ver. [out] Created context version.
         * @param enableDebugContext If true, all debugging capabilities will be enabled
         */
        void create(int width, int height, bool createWindow, double openglVersion, bool enableDebugContext);
        void destroy();

        bool checkSizeChanged() { return false; }

        /**
         * For Windowed double-buffer contexts, this will display back buffer
         * For windowless contexts, this is a no-op
         */
        void swapBuffers();

        /** This needs to be called even if there's no active loop */
        void pollEvents();

        bool windowShouldClose();

        void* windowHandle() const { return (void*)XWindow; }

        void setTitle(const string& title) { }

    private:

        // GL Context via X11
        void XCreate(int width, int height, bool createWindow, double openglVersion, bool enableDebugContext);
        void XDestroy();
        void XSwapBuffers();

        struct FBConfigs
        {
            int Size = 0;
            GLXFBConfig* Data = nullptr;
            FBConfigs(int size, GLXFBConfig* data) : Size(size), Data(data) {}
            FBConfigs(FBConfigs&& fwd) : Size(fwd.Size), Data(fwd.Data) { fwd.Data = nullptr; }
            ~FBConfigs() { if (Data) XFree(Data); }
            FBConfigs(const FBConfigs&) = delete;
        };

        FBConfigs XGetFBConfigs(bool createWindow, int preferredDepth = 24);
        bool XGetVisualInfo(GLXFBConfig config, XVisualInfo& out) const;

        GLXFBConfig XSelectBestFBConfig(bool createWindow);
        void XInitWindow(GLXFBConfig config, int width, int height);
        void XCreatePBuffer(GLXFBConfig config, int width, int height);
        void XCreateContext(GLXFBConfig config, double openglVersion, bool enableDebugContext);
    };

    ////////////////////////////////////////////////////////////////////////////////
}

#endif // __linux__