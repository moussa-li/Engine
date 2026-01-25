#include "Core/ShaderLib.hpp"

#include "Common/Return.hpp"
#include "Common/String.hpp"

// ShaderLib
#include "Basic.shader.h"
#include "Common/HashMap.hpp"

namespace EgLab
{
    template <>
    struct hash<ShaderId>
    {
        size_t operator()(ShaderId x) const
        {
            return static_cast<size_t>(x);
        }
    };

    class ShaderLib::Impl
    {
    public:
        HashMap<ShaderId, String> buffers;
        // String BasicBuffer;
    };

    ShaderLib::ShaderLib() : _data(new Impl)
    {
        _data->buffers.insert({ShaderId::Basic, String(BASIC_SHADER__DATA, BASIC_SHADER__SIZE)});
    }

    ShaderLib::~ShaderLib()
    {
        delete _data;
    }

    Return ShaderLib::getBasicShader(String& buffer)
    {
        Return ret = Return::Succeed;
        buffer = _data->buffers[ShaderId::Basic];
        return ret;
    }

} // namespace EgLab