#pragma once

#include "Common/CommonAPI.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/String.hpp"

namespace EgLab
{
    class CommonAPI StringLineIterator
    {
    public:
        StringLineIterator(String& str);

        void operator++();

        bool operator==(const StringLineIterator&) const;

        bool hasNext() const;

        void getString(const char*& str, size_t& len) const;

    private:
        void getLineEnd();

    private:
        const String& _str;

        DynamicArray<size_t> _lineStart;

        size_t currentIdx;
    };

} // namespace EgLab