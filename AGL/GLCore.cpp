//
// Created by jorma on 24.05.17.
//
#include "GLCore.h"

#if __linux__
    #include "GLContext_Linux.h"
#elif _WIN32
    #include "GLContext_Win32.h"
#endif
#include <vector>
#include "FrameBuffer.h"
#include "SceneRoot.h"
#include <rpp/debugging.h>

namespace AGL
{
    ////////////////////////////////////////////////////////////////////////////////

    struct GLRendererCtx
    {
        static constexpr bool UseFrameBuffer = false;
        static constexpr bool UseDebugContext = true;
        GLContext    Context;
        FrameBuffer  FrameBuf;
        VertexBuffer Triangle;
        Shader       Color3dShader;
        Shader       VertexColor3dShader;
        Shader       Simple3dShader;
        GLInput      Input;

        int Width         = 0;
        int Height        = 0;
        int BytesPerPixel = 0;

        GLRendererCtx(int width, int height, bool createWindow)
        {
            // For proper debugging support, we want to target GL 4.3+
            double openglVersion = 4.3;
            Context.create(width, height, createWindow, openglVersion, UseDebugContext);
            Input.Init(Context.windowHandle());
            Configure();
            LoadDefaultShaders();
        }

        ~GLRendererCtx() = default;

        void Configure()
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND); // enable alpha blending
            if (auto err = glGetErrorStr()) LogError("GL_BLEND error: %s", err);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL); // important to get this right!
            if (auto err = glGetErrorStr()) LogError("GL_DEPTH_TEST error: %s", err);

            //glEnable(GL_TEXTURE_2D); // for iOS
            //if (auto err = glGetErrorStr()) LogError("GL_TEXTURE_2D error: %s", err);

            glEnable(GL_LINE_SMOOTH); // smooth lines if supported
            if (auto err = glGetErrorStr()) LogError("GL_LINE_SMOOTH error: %s", err);

            glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST); // nicest rendering if possible
            if (auto err = glGetErrorStr()) LogError("GL_POLYGON_SMOOTH_HINT error: %s", err);

            //glCullFace(GL_BACK);
            //glEnable(GL_CULL_FACE);
            if (auto err = glGetErrorStr()) LogError("GL_CULL_FACE error: %s", err);

            Width  = Context.width();
            Height = Context.height();
            BytesPerPixel = Context.bytesPerPixel();
            glViewport(0, 0, Width, Height); // always set up default viewport

            if (UseFrameBuffer)
            {
                if (!FrameBuf.initFromDefaultBuffer())
                    FrameBuf.create(Width, Height, GL_RGB);
            }
            else
            {
                LogInfo("Falling back to default Context buffer: %dx%d %d channels", Width, Height, BytesPerPixel);
            }
        }

        void LoadDefaultShaders()
        {
            // So what is this magic? Shader names are defined in EngineShaders.h
            // The shader class automagically falls back to using those shaders. This disables hotloading, obviously...
            // TODO: this is an extremely bad way to load shaders... maybe we can think of something better?? Precompile??
            if (!Color3dShader.loadShader("color3d")) {
                ThrowErr("Failed to load default shader 'color3d'");
            }
            if (!VertexColor3dShader.loadShader("vertexcolor")) {
                ThrowErr("Failed to load default shader 'vertexcolor'");
            }
            if (!Simple3dShader.loadShader("simple3d")) {
                ThrowErr("Failed to load default shader 'simple3d'");
            }
        }

        void UpdateWindowSize()
        {
            if (Context.checkSizeChanged()) {
                Width  = Context.width();
                Height = Context.height();
            }
        }

        void SwapBuffers()
        {
            glFinish(); // block until all rendering completed
            Context.swapBuffers();
            Context.pollEvents();
        }

        bool WindowShouldClose() { return Context.windowShouldClose(); }
    };

    ////////////////////////////////////////////////////////////////////////////////

    GLCore::GLCore(int width, int height, bool createWindow)
        : gl(*new GLRendererCtx(width, height, createWindow)) { }
    GLCore::~GLCore() { delete &gl; }


    void GLCore::SetTitle(const string& title)
    {
        gl.Context.setTitle(title);
    }

    GLInput& GLCore::Input()
    {
        return gl.Input;
    }

    Shader& GLCore::Color3dShader()
    {
        return gl.Color3dShader;
    }

    Shader& GLCore::VertexColor3dShader()
    {
        return gl.VertexColor3dShader;
    }

    Shader& GLCore::Simple3dShader()
    {
        return gl.Simple3dShader;
    }

    int GLCore::ContextWidth() const
    {
        return gl.Width;
    }

    int GLCore::ContextHeight() const
    {
        return gl.Height;
    }

    void GLCore::SaveFrameBuffer(const string& bmpFile) const
    {
        LogInfo("Saving FB %dx%d  %d channels", gl.Width, gl.Height, gl.BytesPerPixel);

        int paddedSize = PaddedImageSize(gl.Width, gl.Height, gl.BytesPerPixel);
        std::vector<uint8_t> image; image.resize(size_t(paddedSize));

        glFinish();
        glFlushErrors();
        glReadPixels(0, 0, gl.Width, gl.Height, GL_BGR, GL_UNSIGNED_BYTE, image.data());
        if (auto err = glGetErrorStr())
            ThrowErr("Fatal: glReadPixels failed: %s", err);

        if (!Texture::saveAsBMP(bmpFile, image.data(), gl.Width, gl.Height, gl.BytesPerPixel))
            ThrowErr("Fatal: failed to save framebuffer to BMP image");

        LogInfo("Saved Framebuffer to '%s'", bmpFile.c_str());
    }

    bool GLCore::WindowShouldClose() const
    {
        return gl.WindowShouldClose();
    }

    void GLCore::SwapBuffers()
    {
        gl.SwapBuffers();
    }

    void GLCore::Clear(float red, float green, float blue)
    {
        gl.UpdateWindowSize();
        glViewport(0, 0, gl.Width, gl.Height);

        glClearColor(red, green, blue, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (SceneRoot)
        {
            if (Camera* camera = SceneRoot->Camera)
            {
                camera->UpdateViewProjection();
            }
        }
    }

    Ray GLCore::Unproject(int x, int y) const
    {
        Camera* camera = SceneRoot->Camera;
        float normX = (2.f * x) / gl.Width - 1.f;
        float normY = 1.f - (2.f * y) / gl.Height;
        Matrix4 unproject = camera->ViewProjection;

        auto rayEye  = unproject * Vector4(normX, normY, -3., 1); // TODO: Magic -3, which makes it work, investigate.
        auto rayClip = unproject * Vector4(normX, normY, 1, 1);

        auto origin = Vector3(rayEye.x  / rayEye.w,  rayEye.y  / rayEye.w,  rayEye.z  / rayEye.w);
        auto target = Vector3(rayClip.x / rayClip.w, rayClip.y / rayClip.w, rayClip.z / rayClip.w * 100.f);

        return { origin, target };
    }

    SceneRoot* GLCore::CreateSceneRoot(string name)
    {
        SceneRoot = std::make_unique<AGL::SceneRoot>(*this, std::move(name));
        return SceneRoot.get();
    }

    void GLCore::UpdateAndRender()
    {
        DeltaTime = FrameTimer.next();
        Input().PollEvents();
        SceneRoot->Update(DeltaTime);
        SceneRoot->Render();
    }

    ////////////////////////////////////////////////////////////////////////////////
}

