//
// Created by Jorma on 25/05/17.
//
#pragma once
#if _WIN32
#include <rpp/delegate.h>
#include "OpenGL.h"
#include "AGLConfig.h"
#include <Objbase.h> // CoInitialize
#include <GLFW/glfw3.h>
#ifndef USING_GLFW
    #define USING_GLFW 1
#endif

namespace AGL
{
    using std::string;
    ////////////////////////////////////////////////////////////////////////////////

    class AGL_API GLContext
    {
        int Width            = 0;
        int Height           = 0;
        int BytesPerPixel    = 0;
        GLFWwindow* Window   = nullptr;
        bool GLFWInitialized = false;

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

        bool checkSizeChanged();

        /**
         * For Windowed double-buffer contexts, this will display back buffer
         * For windowless contexts, this is a no-op
         */
        void swapBuffers();

        /** This needs to be called even if there's no active loop */
        void pollEvents();

        bool windowShouldClose();

        void* windowHandle() const { return Window; }

        void setTitle(const string& title);
    };

    ////////////////////////////////////////////////////////////////////////////////
}

#endif // _WIN32