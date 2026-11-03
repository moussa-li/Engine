#include "Common/StringLineIterator.hpp"

namespace EgLab::Common
{
    StringLineIterator::StringLineIterator(String& str) : _str(str)
    {
        _lineStart.pushBack(0);
        currentIdx = 0;
        getLineEnd();
    }

    StringLineIterator::~StringLineIterator()
    {
    }

    void StringLineIterator::getLineEnd()
    {
        const char* data = _str.c_str();
        size_t len = _str.size();
        if (len == 0) return;

        for (size_t i = 0; i < len; ++i)
        {
            if (data[i] == '\n' && i + 1 < len)
            {
                _lineStart.pushBack(i + 1);
            }
        }

        _lineStart.pushBack(len);
    }

    bool StringLineIterator::hasNext() const
    {
        return currentIdx + 1 < _lineStart.size();
    }

    void StringLineIterator::operator++()
    {
        currentIdx++;
    }

    bool StringLineIterator::operator==(const StringLineIterator& other) const
    {
        return other.currentIdx == currentIdx;
    }

    void StringLineIterator::getString(const char*& str, size_t& len) const
    {
        const char* s = _str.c_str();
        str = s + _lineStart[currentIdx];
        len = _lineStart[currentIdx + 1] - _lineStart[currentIdx];
    }
} // namespace EgLab::Common