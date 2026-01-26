#include "Common/StringLineIterator.hpp"

#include <emmintrin.h>

namespace EgLab
{
    StringLineIterator::StringLineIterator(String& str) : _str(str)
    {
        _lineStart.pushBack(0);
        currentIdx = 0;
        getLineEnd();
    }

    void StringLineIterator::getLineEnd()
    {
        const char* data = _str.c_str();
        size_t len = _str.size();
        if (_str.size() < 2) return;

        const __m128i cr_char = _mm_set1_epi8('\r');
        const __m128i nl_char = _mm_set1_epi8('\n');

        size_t i = 0;
        for (; i + 16 <= len; i += 16)
        {
            __m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i));
            __m128i cmp = _mm_cmpeq_epi8(block, cr_char);

            int mask = _mm_movemask_epi8(cmp);

            while (mask != 0)
            {
                int bit_pos = __builtin_ctz(mask);
                size_t pos = i + bit_pos;
                if (pos + 1 < len && data[pos + 1] == '\n')
                {
                    _lineStart.pushBack(pos + 2);
                }
                mask &= ~(1 << bit_pos);
            }
        }

        for (; i + 1 < len; ++i)
        {
            if (data[i] == '\r' && data[i + 1] == '\n')
            {
                _lineStart.pushBack(i + 2);
            }
        }
        if (_lineStart.back() >= len) _lineStart.popBack();
        _lineStart.pushBack(_str.size() - 1); // an extra '\0' was added.
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
} // namespace EgLab