//
// Created by jorma on 24.05.17.
//
#if __linux__
#include "GLContext_Linux.h"
#include <rpp/strview.h>
#include <cmath>

namespace AGL
{
    ////////////////////////////////////////////////////////////////////////////////

    GLContext::GLContext()
    {
    }

    GLContext::~GLContext()
    {
        destroy();
    }

    void GLContext::create(int width, int height, bool createWindow, double openglVersion, bool enableDebugContext)
    {
        XCreate(width, height, createWindow, openglVersion, enableDebugContext);
    }

    void GLContext::destroy()
    {
        XDestroy();
    }

    void GLContext::swapBuffers()
    {
        XSwapBuffers();
    }

    void GLContext::pollEvents()
    {
        // @TODO: Implement this
    }

    bool GLContext::windowShouldClose()
    {
        if (XWindow)
        {
            // @TODO: Implement this
        }
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////

    // Install an X error handler so the application won't exit if GL <VERSION>
    // context allocation fails.
    //
    // Note this error handler is global.  All display connections in all threads
    // of a process use the same error handler, so be sure to guard against other
    // threads issuing X commands while this code is running.
    static bool GLXErrorOccurred;
    static int GLXErrorCode;
    static XErrorHandler GLXOldErrHandler;
    static void GLXClearErrors()
    {
        GLXErrorOccurred = false;
        GLXErrorCode = 0;
    }
    static void GLXSetErrorHandler(XErrorHandler handler = nullptr)
    {
        GLXClearErrors();
        GLXOldErrHandler = XSetErrorHandler(handler ? handler : GLXOldErrHandler);
    }

    void GLContext::XCreate(int width, int height, bool createWindow, double openglVersion, bool enableDebugContext)
    {
        GLXSetErrorHandler([](Display*, XErrorEvent* evt) {
            GLXErrorOccurred = true;
            GLXErrorCode = evt->error_code;
            return 0;
        });
        try
        {
            printf("============================================================\n");
            printf("Creating %s OpenGL Context ... \n", createWindow ? "Windowed" : "Headless");
            XDisplay = XOpenDisplay(NULL);
            if (!XDisplay) ThrowErr("Fatal: Failed to open X display");

            GLXFBConfig bestConfig = XSelectBestFBConfig(createWindow);
            if (createWindow) XInitWindow(bestConfig, width, height);
            else              XCreatePBuffer(bestConfig, width, height);
            XCreateContext(bestConfig, openglVersion, enableDebugContext);

            if (glXIsDirect(XDisplay, XGLctx)) // Verify that context is a direct context
                printf("SUCCESS: Direct GLX rendering context obtained\n");
            else LogError("WARNING: Indirect GLX rendering context was created");

            printf("MakeCurrent GLX drawable: ");
            if (createWindow)
            {
                printf("XWindow ... ");
                glXMakeCurrent(XDisplay, XWindow, XGLctx);
            }
            else
            {
                printf("XPBuffer ... ");
                glXMakeCurrent(XDisplay, XPBuffer, XGLctx);
            }
            printf("OK\n");

            const char* glversion = (char*)glGetString(GL_VERSION);
            if (!glversion) ThrowErr("Failed to get opengl version. Context is probably invalid");

            float version = rpp::strview{ glversion }.next_float();
            printf("GL Version  = %.1f (%s)\n", version, glversion);
            printf("GL Vendor   = %s\n", glGetString(GL_VENDOR));
            printf("GL Renderer = %s\n", glGetString(GL_RENDERER));
            printf("GL GLSL Ver = %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
            //printf("GL Exts     = %s\n", glGetString(GL_EXTENSIONS));
            GLXSetErrorHandler();

            glInitialize(enableDebugContext);
            printf("============================================================\n");
        }
        catch (...)
        {
            GLXSetErrorHandler();
            destroy();
            printf("============================================================\n");
            throw;
        }
    }

    void GLContext::XDestroy()
    {
        if (!XDisplay)
            return;
        printf("Destroying OpenGL Context\n");

        glXMakeCurrent(XDisplay, 0, 0);
        if (XPBuffer)  { glXDestroyPbuffer(XDisplay, XPBuffer), XPBuffer  = 0; }
        if (XGLctx)    { glXDestroyContext(XDisplay, XGLctx),   XGLctx    = 0; }
        if (XWindow)   { XDestroyWindow(XDisplay, XWindow),     XWindow   = 0; }
        if (XColorMap) { XFreeColormap(XDisplay, XColorMap),    XColorMap = 0; }
        XCloseDisplay(XDisplay), XDisplay = 0;
        Width = Height = BytesPerPixel = 0;
    }

    void GLContext::XSwapBuffers()
    {
        if (XWindow) {
            glXSwapBuffers(XDisplay, XWindow);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////

    GLContext::FBConfigs GLContext::XGetFBConfigs(bool createWindow, int preferredDepth)
    {
        // Get a matching FB config
        // Look in the /usr/gfx/ucode/MGRAS/vof/
        // directory for display configurations with the _pbuf suffix.  Use
        // setmon -x <vof> to configure your X server and display for pbuffers.
        static int attributes[] = {
                GLX_X_RENDERABLE , True,
                GLX_DRAWABLE_TYPE, createWindow ? GLX_WINDOW_BIT : GLX_PBUFFER_BIT,
                GLX_RENDER_TYPE  , GLX_RGBA_BIT,
                GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
                GLX_RED_SIZE     , 8,
                GLX_GREEN_SIZE   , 8,
                GLX_BLUE_SIZE    , 8,
                GLX_ALPHA_SIZE   , preferredDepth > 24 ? 8 : 0,
                GLX_DEPTH_SIZE   , 16, // 16-bit depth
                GLX_STENCIL_SIZE , 8,
                GLX_DOUBLEBUFFER , True,
                //GLX_SAMPLE_BUFFERS  , 1, // number of required AA multisample buffers
                //GLX_SAMPLES         , 4, // number of AA multisamples per pixel
                None
        };

        int glxMajor = 0, glxMinor = 0; // FBConfigs were added in GLX version 1.3.
        if (!glXQueryVersion(XDisplay, &glxMajor, &glxMinor)
            || glxMajor < 1 || (glxMajor == 1 && glxMinor < 3))
        {
            ThrowErr("Fatal: Invalid GLX version (%d.%d), FBConfigs are not supported", glxMajor, glxMinor);
        }

        int numConfigs = 0;
        GLXFBConfig* configs = glXChooseFBConfig(XDisplay, DefaultScreen(XDisplay), attributes, &numConfigs);
        if (!configs)
        {
            ThrowErr("Fatal: Failed to get GLX FrameBuffer Configurations");
        }
        return { numConfigs, configs };
    }

    bool GLContext::XGetVisualInfo(GLXFBConfig config, XVisualInfo& out) const
    {
        XVisualInfo* vi = glXGetVisualFromFBConfig(XDisplay, config);
        if (!vi) return false;
        out = *vi;
        XFree(vi);
        return true;
    }
    static const char* XVIClassName(XVisualInfo& vi) {
        static const char* mapping[] = { "StaticGray","GrayScale","StaticColor","PseudoColor","TrueColor","DirectColor" };
        return ((uint)vi.c_class <= DirectColor) ? mapping[vi.c_class] : "InvalidColor";
    };
    static const char* XVIPixelFormat(XVisualInfo& vi) {
        if (vi.depth == 24) return vi.blue_mask < vi.red_mask ? "BGR" : "RGB";
        else if (vi.depth == 32) return vi.blue_mask < vi.red_mask ? "BGRA" : "RGBA";
        return "unknown";
    };

    GLXFBConfig GLContext::XSelectBestFBConfig(bool createWindow)
    {
        constexpr int preferredDepth = 24; // prefer 24-bit RGB
        FBConfigs configs = XGetFBConfigs(createWindow, preferredDepth);

        printf("FrameBufferConfigs found: %d. Selecting best:\n", configs.Size);
        Visual* defaultVisual = XDefaultVisual(XDisplay, 0);
        XVisualInfo best = { 0 };
        XVisualInfo info;

        GLXFBConfig bestConfig = nullptr;
        for (int i = configs.Size - 1; i >= 0; --i) // best configs are last, so iterate backwards
        {
            if (!XGetVisualInfo(configs.Data[i], info))
                continue;
            printf("  FBC %2d: ID 0x%-3lx %2d-bit %-4s %s\n", i, info.visualid, info.depth, XVIPixelFormat(info), XVIClassName(info));
            if (info.visual == defaultVisual) printf("    ^ default visual\n");
            if ((best.depth == 0 || info.visual == defaultVisual) && info.depth == preferredDepth) {
                best = info;
                bestConfig = configs.Data[i];
            }
        }

        if (!bestConfig)
        {
            ThrowErr("Fatal: Failed to find a valid FBConfig");
            return nullptr;
        }

        BytesPerPixel = best.depth / 8;
        printf("  [picked FBC 0x%-3lx %2d-bit %-4s %s]\n", best.visualid, best.depth, XVIPixelFormat(best), XVIClassName(best));
        return bestConfig;
    }

    void GLContext::XInitWindow(GLXFBConfig config, int width, int height)
    {
        XVisualInfo vi; XGetVisualInfo(config, vi);

        printf("Chosen visual ID = 0x%lx\n", vi.visualid);
        XSetWindowAttributes swa;
        swa.colormap = XColorMap = XCreateColormap(XDisplay, RootWindow(XDisplay, vi.screen), vi.visual, AllocNone);
        swa.background_pixmap = None;
        swa.border_pixel = 0;
        swa.event_mask = StructureNotifyMask;

        printf("Creating window... ");
        XWindow = XCreateWindow(XDisplay, RootWindow(XDisplay, vi.screen),
            0, 0, (uint)width, (uint)height, 0, vi.depth, InputOutput,
            vi.visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
        if (!XWindow) {
            ThrowErr("Fatal: Failed to create window.");
            return;
        }
        printf("OK\n");
        XStoreName(XDisplay, XWindow, "Window");

        printf("Mapping window... ");
        XMapWindow(XDisplay, XWindow);
        printf("OK\n");
    }

    void GLContext::XCreatePBuffer(GLXFBConfig config, int width, int height)
    {
        printf("Creating XPBuffer... ");
        int attributes[] = {
            GLX_PBUFFER_WIDTH,      width,
            GLX_PBUFFER_HEIGHT,     height,
            GLX_PRESERVED_CONTENTS, False,
            GLX_LARGEST_PBUFFER,    True, // if allocation fails, fallback to whatever
            None,
        };
        XPBuffer = glXCreatePbuffer(XDisplay, config, attributes);
        if (!XPBuffer) {
            ThrowErr("Fatal: Failed to create XPBuffer %dx%d.", width, height);
            return;
        }
        printf("OK: %dx%d\n", width, height);
        Width = width;
        Height = height;
    }

    void GLContext::XCreateContext(GLXFBConfig config, double openglVersion, bool enableDebugContext)
    {
        int major = (int)openglVersion;
        int minor = (int)round((openglVersion - major) * 10);
        int attributes[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, major,
            GLX_CONTEXT_MINOR_VERSION_ARB, minor,
            GLX_CONTEXT_FLAGS_ARB, enableDebugContext ? GLX_CONTEXT_DEBUG_BIT_ARB : 0,
            None
        };

        // Get the default screen's GLX extension list
        //auto glxExts = glXQueryExtensionsString(XDisplay, DefaultScreen(XDisplay));
        //printf("GLX EXTENSIONS: %s\n", glxExts);
        //bool isCreateCtxSupported = glIsExtAvailable(glxExts, "GLX_ARB_create_context");

        using glxCreateCtxAttribs = GLXContext(*)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
        auto createContext = (glxCreateCtxAttribs)glXGetProcAddressARB((GLubyte*)"glXCreateContextAttribsARB");

        constexpr bool preferDirectContext = true;

        // Check for the GLX_ARB_create_context extension string and the function.
        // If either is not present, use GLX 1.3 context creation method.
        if (createContext)
        {
            auto tryCreateContext = [&](int openglMajor, int openglMinor)
            {
                printf("Trying to create GL %d.%d context...", openglMajor, openglMinor);
                attributes[0] = openglMajor;
                attributes[2] = openglMinor;

                GLXClearErrors();
                XGLctx = createContext(XDisplay, config, 0, preferDirectContext, attributes);
                XSync(XDisplay, False); // Sync to ensure any errors generated are processed.
                printf("%s\n", XGLctx != nullptr ? "SUCCESS" : "FAILED");
                return XGLctx != nullptr;
            };

            if (!tryCreateContext(major, minor) && !tryCreateContext(2,0))
            {
            }
        }

        if (!XGLctx)
        {
            LogWarning("glXCreateContextAttribsARB() failed... using glXCreateNewContext");
            GLXClearErrors();
            XGLctx = glXCreateNewContext(XDisplay, config, GLX_RGBA_TYPE, 0, preferDirectContext);
            XSync(XDisplay, False);
        }

        if (GLXErrorOccurred || !XGLctx) {
            ThrowErr("Fatal: Failed to create an OpenGL context (errcode %d)", GLXErrorCode);
            return;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
}
#endif // __linux__
