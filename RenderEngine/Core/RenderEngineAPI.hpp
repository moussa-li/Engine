#pragma once


#ifdef _WIN32
    #ifdef RenderEngine
        #define RenderEngineAPI __declspec(dllexport)
    #else
        #define RenderEngineAPI __declspec(dllimport)
    #endif
#else // Linux
    #define CommonAPI __attribute__((visibility("default")))
#endif