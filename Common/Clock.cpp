#include "Common/Clock.hpp"
#include "Common/StringTools.hpp"

namespace EgLab
{
    std::chrono::system_clock::time_point EgLab::Clock::now()
    {
        return std::chrono::system_clock::now();
    }

    void Clock::time(int &hour, int &minute, int &second)
    {
        auto nowTime = std::chrono::system_clock::to_time_t(now());
        std::tm* ltm = std::localtime(&nowTime);
        hour = ltm->tm_hour;
        minute = ltm->tm_min;
        second = ltm->tm_sec;
    }

    void Clock::time(int& hour, int& minute)
    {
        auto nowTime = std::chrono::system_clock::to_time_t(now());
        std::tm* ltm = std::localtime(&nowTime);
        hour = ltm->tm_hour;
        minute = ltm->tm_min;
    }

    void Clock::time(String& hour, String& minute, String& second)
    {
        auto nowTime = std::chrono::system_clock::to_time_t(now());
        std::tm* ltm = std::localtime(&nowTime);
        if (ltm->tm_hour < 10)
            hour = "0" + EgLab::ToString(ltm->tm_hour);
        else
            hour = EgLab::ToString(ltm->tm_hour);

        if(ltm->tm_min < 10)
            minute = "0" + EgLab::ToString(ltm->tm_min);
        else
            minute = EgLab::ToString(ltm->tm_min);

        if (ltm->tm_sec < 10)
            second = "0" + EgLab::ToString(ltm->tm_sec);
        else
            second = EgLab::ToString(ltm->tm_sec);
    }

    std::chrono::high_resolution_clock::time_point Clock::timestamp()
    {
        return std::chrono::high_resolution_clock::now();
    }

    void Clock::delta_time(std::chrono::high_resolution_clock::time_point t1, std::chrono::high_resolution_clock::time_point t2, int& hour, int& minute, int& second)
    {
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1);
        hour = std::chrono::duration_cast<std::chrono::hours>(duration).count();
        minute = std::chrono::duration_cast<std::chrono::minutes>(duration%std::chrono::hours(1)).count();
        second = std::chrono::duration_cast<std::chrono::seconds>(duration%std::chrono::minutes(1)).count();
    }

    void Clock::delta_time(std::chrono::high_resolution_clock::time_point t1, std::chrono::high_resolution_clock::time_point t2, int& microsecond)
    {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        microsecond = duration.count();
    }

    double Clock::delta_time(std::chrono::high_resolution_clock::time_point t1, std::chrono::high_resolution_clock::time_point t2)
    {
        auto duration = std::chrono::duration<double>(t2 - t1);
        return duration.count();
    }

}
