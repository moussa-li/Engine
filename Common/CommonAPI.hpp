#pragma once


#ifdef _WIN32
    #ifdef EgLabCommon
        #define CommonAPI __declspec(dllexport)
    #else
        #define CommonAPI __declspec(dllimport)
    #endif
#else //Linux
        #define CommonAPI __attribute__((visibility("default")))
#endif

