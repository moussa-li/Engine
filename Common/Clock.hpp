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
#include <Common/String.hpp>
#include <chrono>

namespace EgLab
{
    class Clock
    {
    public:
        Clock() = default;

        /************************************************************************/
        /*                The use of low-precision timing functions such as logs */
        /************************************************************************/
        /**
         * The current time is returned as the system clock time point
         */
        static std::chrono::system_clock::time_point now();

        /**
         * 当前时间
         */
        static void time(int &hour, int &minute);
        static void time(int &hour, int &minute, int &second);
        static void time(String &hour, String &minute, String &second);

        /************************************************************************/
        /*  High-precision timing functions such as performance analysis are used */
        /************************************************************************/
        /**
         * The return timestamp is returned with high-precision time point
         */
        static std::chrono::high_resolution_clock::time_point timestamp();

        /**
         * delta time
         * t1 to t2 time
         * t2 - t1
         */
        static void delta_time(std::chrono::high_resolution_clock::time_point t1,
                               std::chrono::high_resolution_clock::time_point t2, int &hour,
                               int &minute, int &second);

        static void delta_time(std::chrono::high_resolution_clock::time_point t1,
                               std::chrono::high_resolution_clock::time_point t2, int &microsecond);

        static double delta_time(std::chrono::high_resolution_clock::time_point t1,
                                 std::chrono::high_resolution_clock::time_point t2);

    private:
    };

} // namespace EgLab