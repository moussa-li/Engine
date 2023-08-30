#pragma once

#include <iostream>

#include <cstdio>

#define __DLL_EXPORTS__
#ifdef __DLL_EXPORTS__
#define DLLAPI  __declspec(dllexport)
#else
#define DLLAPI __declspec(dllimport)
#endif



#define ASSERT(x) if(!(x)) __debugbreak();

#define GLCall(x) GLClearError();\
                  x;\
                  ASSERT(GLLogCall(#x, __FILE__, __LINE__))

DLLAPI void GLClearError();

DLLAPI bool GLLogCall(const char* function, const char* file, int line);

namespace Log{

    enum LogLevel
    {
        Error = 0,
        Warning,
        Info,
        Debug
    };
    
    static LogLevel Level = Debug;
    void Log(LogLevel l, std::string message);
    void Log(LogLevel l, const char* function, const char* file, int line, std::string message);

}
