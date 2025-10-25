#pragma once
/*!
 * @file Clock.h
 * @brief Clock Tool
 * @details TODO:long description
 * @mainpage
 * @author Moussa_Li
 * @version 1.0
 * @date 2025/09/27
 * @license GPLV2
 *
*/
#include <chrono>
#include <Common/String.hpp>

namespace EgLab
{
    class Clock {
    public:
        Clock() = default;

        /************************************************************************/
        /*                       日志等低精度计时功能使用                           */
        /************************************************************************/
        /**
         * 当前时间以系统时钟time_point返回
         */
        static std::chrono::system_clock::time_point now();

        /**
         * 当前时间
         */
        static void time(int &hour, int &minute);
        static void time(int &hour, int &minute, int &second);
        static void time(String& hour, String& minute, String& second);

        /************************************************************************/
        /*                        性能分析等高精度计时功能使用                      */
        /************************************************************************/
        /**
         * 返回时间戳以高精度time_point返回
         */
        static std::chrono::high_resolution_clock::time_point timestamp();

        /**
         * 时间增量
         * t1 到 t2 的时间差
         * t2 - t1
         */
        static void delta_time(std::chrono::high_resolution_clock::time_point t1,
            std::chrono::high_resolution_clock::time_point t2,
            int &hour, int &minute, int &second);

        static void delta_time(std::chrono::high_resolution_clock::time_point t1,
            std::chrono::high_resolution_clock::time_point t2,
            int &microsecond);

        static double delta_time(std::chrono::high_resolution_clock::time_point t1,
            std::chrono::high_resolution_clock::time_point t2);


    private:
    };

}