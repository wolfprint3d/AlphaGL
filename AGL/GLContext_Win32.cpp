#if _WIN32
#include "GLContext_Win32.h"
#include <rpp/strview.h>

///////////////////////////////////////////////////////////////////////////////////////////////

namespace AGL
{
    GLContext::GLContext()
    {
    }

    GLContext::~GLContext()
    {
        destroy();
    }

    void GLContext::create(int width, int height, bool createWindow, double openglVersion, bool enableDebugContext)
    {
        try
        {
            static bool coinit = [] {
                CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
                return true;
            }();

            LogInfo("============================================================");
            LogInfo("Creating %s OpenGL Context ... ", createWindow ? "Windowed" : "Headless");

            if (!glfwInit()) ThrowErr("GLFW init failed");
            GLFWInitialized = true;

            if (!createWindow)
                glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

            int major = (int)openglVersion;
            int minor = (int)round((openglVersion - major) * 10);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
            //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // disable deprecated features ?
            //glfwWindowHint(GLFW_SAMPLES, 4); // 4xMSAA

            if (enableDebugContext)
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // DEBUG OpenGL Driver

            Window = glfwCreateWindow(width, height, "GLRenderer", nullptr, nullptr);
            if (!Window)
                ThrowErr("Failed to create GLFW window");

            glfwSetErrorCallback([](int error, const char* string) {
                ThrowErr("GLFW error: %s", string);
            });
            glfwMakeContextCurrent(Window);

            const char* glversion = (char*)glGetString(GL_VERSION);
            if (!glversion) ThrowErr("Failed to get opengl version. Context is probably invalid");

            Width         = width;
            Height        = height;
            BytesPerPixel = 3;

            float version = rpp::strview{ glversion }.next_float();
            LogInfo("GL Version  = %.1f (%s)", version, glversion);
            LogInfo("GL Vendor   = %s", (char*)glGetString(GL_VENDOR));
            LogInfo("GL Renderer = %s", (char*)glGetString(GL_RENDERER));
            LogInfo("GL GLSL Ver = %s", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
            //LogInfo("GL Exts     = %s", (char*)glGetString(GL_EXTENSIONS));

            glInitialize(enableDebugContext);
            LogInfo("============================================================");
        }
        catch (...)
        {
            destroy();
            LogInfo("============================================================");
            throw;
        }
    }

    void GLContext::destroy()
    {
        if (Window)
        {
            glfwDestroyWindow(Window);
            Window = nullptr;
            Width = Height = BytesPerPixel = 0;
        }
        if (GLFWInitialized)
        {
            glfwTerminate();
            GLFWInitialized = false;
        }
    }

    bool GLContext::checkSizeChanged()
    {
        int newWidth, newHeight;
        glfwGetWindowSize(Window, &newWidth, &newHeight);
        if (Width != newWidth || Height != newHeight)
        {
            Width  = newWidth;
            Height = newHeight;
            return true;
        }
        return false;
    }

    void GLContext::swapBuffers()
    {
        glfwSwapBuffers(Window);
    }

    void GLContext::pollEvents()
    {
        glfwPollEvents();
    }

    bool GLContext::windowShouldClose()
    {
        return glfwWindowShouldClose(Window) == GLFW_TRUE;
    }

    void GLContext::setTitle(const string& title)
    {
        glfwSetWindowTitle(Window, title.c_str());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////

#endif // _WIN32
