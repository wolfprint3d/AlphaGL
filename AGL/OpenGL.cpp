//
// Created by jorma on 5/25/17.
//
#include "OpenGL.h"
#include <stdexcept>
#include <cstring>

namespace AGL
{
    ////////////////////////////////////////////////////////////////////////////////

    static void GLAPIENTRY OpenGLDebugHandler(GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, const void* userParam)
    {
        const char* typeString;
        switch (type) {
        case GL_DEBUG_TYPE_ERROR:               typeString = "GL_ERROR";               break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeString = "GL_DEPRECATED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeString = "GL_UNDEFINED_BEHAVIOR";  break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeString = "GL_PORTABILITY";         break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeString = "GL_PERFORMANCE";         break;
        default: return; // dont't care about info strings
        }

        if (severity == GL_DEBUG_SEVERITY_HIGH)
            LogError("%s (%d):\n%s", typeString, id, message);
        else
            LogWarning("%s (%d):\n%s", typeString, id, message);
    }

    void glInitialize(bool withDebuggingEnabled)
    {
    #ifdef __glew_h__
        ::glewExperimental = 1; // enable loading experimental OpenGL features
        if (GLenum status = glewInit()) { // init GL extension wrangler
            ThrowErr("Fatal: GLEW init failed: %s", glewGetErrorString(status));
        }
    #endif
        if (withDebuggingEnabled && glDebugMessageCallback)
        {
            LogInfo("Enabling OpenGL DEBUG callbacks\n");
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(OpenGLDebugHandler, nullptr);
            GLuint unusedIds = 0;
            glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_LOW, 0, &unusedIds, true);
        }
        glFlushErrors();
    }

    bool glIsExtAvailable(const void* extList, const char* extension)
    {
        if (extList == nullptr)
            return false;

        auto* where = strchr(extension, ' '); // Extension names should not have spaces.
        if (where || *extension == '\0')
            return false;

        // It takes a bit of care to be fool-proof about parsing OpenGL extensions string.
        auto extLength = strlen(extension);
        for (auto* start = (const char*)extList; (where = strstr(start, extension));) {
            auto* end = where + extLength;
            if ((where == start || *(where - 1) == ' ') && (*end == ' ' || *end == '\0'))
                return true;
            start = end;
        }
        return false;
    }

    void glFlushErrors()
    {
        for (int i = 0; i < 100 && glGetError(); ++i)
            ; // ignore all errors
    }

    const char* glGetErrorStr()
    {
        GLenum err = glGetError();
        if (err == GL_NO_ERROR)
            return NULL;

        static char buffer[256];
        const char* msg;
        switch (err)
        {
        case GL_INVALID_ENUM:      msg = "GL_INVALID_ENUM";      break;
        case GL_INVALID_VALUE:     msg = "GL_INVALID_VALUE";     break;
        case GL_INVALID_OPERATION: msg = "GL_INVALID_OPERATION"; break;
        case GL_OUT_OF_MEMORY:     msg = "GL_OUT_OF_MEMORY";     break;
        default:                   msg = "";                     break;
        }
        sprintf(buffer, "%s(%d)", msg, err);
        return buffer;
    }

    ////////////////////////////////////////////////////////////////////////////////
}
