#include "Common/StringTools.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace EgLab
{
    void ToLower(std::string& str)
    {
        for (auto& c : str)
        {
            c = std::tolower(c);
        }
    }

    void ToUpper(std::string& str)
    {
        for (auto& c : str)
        {
            c = std::toupper(c);
        }
    }

    std::vector<std::string> SplitStr(const std::string& str, const char splitSymbol)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;

        while (getline(ss, token, splitSymbol)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::vector<std::string> SplitStrByCommas(const std::string& str)
    {
        return SplitStr(str, ',');
    }

    std::vector<std::string> SplitStrByDot(const std::string& str)
    {
        return SplitStr(str, '.');
    }

    int ToInt(const std::string& str)
    {
        return std::atoi(str.c_str());
    }

    std::vector<int> ToInt(const std::vector<std::string>& str)
    {
        std::vector<int> ints;
        for (auto& s : str) {
            ints.push_back(ToInt(s));
        }
        return ints;
    }

    float ToFloat(const std::string& str)
    {
        return (float)std::atof(str.c_str());
    }

    double ToDouble(const std::string& str)
    {
        return std::stod(str);
    }

    std::string gb2312ToUtf8(const std::string& gb2312)
    {
#ifdef _WIN32   
        int len = MultiByteToWideChar(CP_ACP, 0, gb2312.c_str(), -1, NULL, 0);
        wchar_t* wstr = new wchar_t[len + 1];
        memset(wstr, 0, len + 1);
        MultiByteToWideChar(CP_ACP, 0, gb2312.c_str(), -1, wstr, len);
        len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
        std::string utf8(len,0);
        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &utf8[0], len, NULL, NULL);
        if (wstr) delete[] wstr;
        return utf8;
#else
        return gb2312;
#endif
    }

}


