#pragma once
#include "Common/CommonAPI.hpp"
#include "Common/String.hpp"

#include <vector>
#include <sstream>

namespace EgLab
{
    /**
     * transfer data to string
     * 
     * @param t
     * @return 
     */
    template<typename T>
    inline String ToString(const T& t)
    {
        std::stringstream ss;
        ss << t;
        return String(ss.str().c_str());
    }

    CommonAPI void ToLower(std::string&);

    CommonAPI void ToUpper(std::string&);

    CommonAPI int ToInt(const std::string&);

    CommonAPI std::vector<int> ToInt(const std::vector<std::string>&);

    CommonAPI float ToFloat(const std::string&);

    CommonAPI double ToDouble(const std::string&);

    CommonAPI std::vector<std::string> SplitStr(const std::string&, const char splitSymbol);

    CommonAPI std::vector<std::string> SplitStrByCommas(const std::string&);

    CommonAPI std::vector<std::string> SplitStrByDot(const std::string&);

    CommonAPI std::string gb2312ToUtf8(const std::string&);
}
