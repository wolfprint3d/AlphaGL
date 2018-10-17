#pragma once

#ifndef AGL_API
    #if _MSC_VER
        #define AGL_API __declspec(dllexport)
    #else // clang/gcc
        #define AGL_API __attribute__((visibility("default")))
    #endif
#endif
