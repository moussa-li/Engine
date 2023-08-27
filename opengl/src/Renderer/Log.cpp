#include "Log.h"
#include <GL/glew.h>


namespace Log{

    #define NONE          "\033[0m   "
    #define BLACK         "\033[0;30m"
    #define DARK_GRAY     "\033[1;30m"
    #define BLUE          "\033[0;34m"
    #define LIGHT_BLUE    "\033[1;34m"
    #define GREEN         "\033[0;32m"
    #define LIGHT_GREEN - "\033[1;32m"
    #define CYAN          "\033[0;36m"
    #define LIGHT_CYAN    "\033[1;36m"
    #define RED           "\033[0;31m"
    #define LIGHT_RED     "\033[1;31m"
    #define PURPLE        "\033[0;35m"
    #define LIGHT_PURPLE  "\033[1;35m"
    #define BROWN         "\033[0;33m"
    #define YELLOW        "\033[1;33m"
    #define LIGHT_GRAY    "\033[0;37m"
    #define WHITE         "\033[1;37m"


    #define END "\033[0m"   //关闭所有属性      
    #define HIGHLIGHT "\033[1m"   //设置高亮度      
    #define UNDERLINE "\033[4m"   //下划线      
    #define SHINING "\033[5m"   //闪烁      
/*
    #define \033[7m   //反显      
    #define \033[8m   //消隐    

    
    #define \033[30m   --   \033[37m   //设置前景色     
    #define \033[40m   --   \033[47m   //设置背景色      

    #define \033[nA   //光标上移n行     
    #define \03[nB    //光标下移n行      
    #define \033[nC   //光标右移n行     
    #define \033[nD   //光标左移n行
    */

    const std::string LevelItem[] = {
        "Error",
        "Warning",
        "Info",
        "Debug"
    };
    
    void Log(LogLevel l, std::string message)
    {
        if (l <= Level)
        {
            std::string EchoLevel =  LevelItem[Level];

            printf("%s[ %s ]%s : %s \n", LIGHT_BLUE,EchoLevel.c_str(),END, (message).c_str());
        }
    }
    
    void Log(LogLevel l,const char* function, const char* file, int line, std::string message)
    {
        if (l <= Level)
        {
            std::string EchoLevel =  LevelItem[Level];
            printf("%s[ %s ]%s (%s) %s[%s %s : %d]%s \n", LIGHT_BLUE,EchoLevel.c_str(),END, message.c_str(),YELLOW, file, function, line,END);
        }
    }
    
}



void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char * function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << ") : " << function << file << ":" << line << std::endl;
        return false;
    }
    return true;
}
