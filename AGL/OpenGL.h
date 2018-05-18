//
// Created by Jorma on 25/05/17.
//
#pragma once
#include "glew.h"
#include <cstdio>
#include <rpp/debugging.h>

namespace AGL
{
    ////////////////////////////////////////////////////////////////////////////////

    /**
     * Initializes OpenGL and optionally enables OpenGL 4.3+ debugging capabilities.
     * Will throw a detailed exception if initialization fails
     */
    void glInitialize(bool withDebuggingEnabled);

    /**
     * @return TRUE if opengl extension is defined in the provided extensions list
     */
    bool glIsExtAvailable(const void* extList, const char* extension);

    /**
     * @brief Flush any pending OpenGL errors
     */
    void glFlushErrors();

    /**
     * @brief Calls glGetError() and translates the error code into a readable string
     * @return NULL if there is no error, or an error string
     */
    const char* glGetErrorStr();

    ////////////////////////////////////////////////////////////////////////////////
}

