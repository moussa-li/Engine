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
        /*                       ��־�ȵ;��ȼ�ʱ����ʹ��                           */
        /************************************************************************/
        /**
         * ��ǰʱ����ϵͳʱ��time_point����
         */
        static std::chrono::system_clock::time_point now();

        /**
         * ��ǰʱ��
         */
        static void time(int &hour, int &minute);
        static void time(int &hour, int &minute, int &second);
        static void time(String& hour, String& minute, String& second);

        /************************************************************************/
        /*                        ���ܷ����ȸ߾��ȼ�ʱ����ʹ��                      */
        /************************************************************************/
        /**
         * ����ʱ����Ը߾���time_point����
         */
        static std::chrono::high_resolution_clock::time_point timestamp();

        /**
         * ʱ������
         * t1 �� t2 ��ʱ���
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