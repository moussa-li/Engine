#include "Core/ShaderLib.hpp"

#include "Common/Return.hpp"
#include "Common/String.hpp"

// ShaderLib
#include "Basic.shader.h"
#include "Common/HashMap.hpp"
#include "Easy.shader.h"
#include "Node.shader.h"


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
        _data->buffers.insert({ShaderId::Node, String(NODE_SHADER__DATA, NODE_SHADER__SIZE)});
        _data->buffers.insert({ShaderId::Easy, String(EASY_SHADER__DATA, EASY_SHADER__SIZE)});
    }

    ShaderLib::~ShaderLib()
    {
        delete _data;
    }

    Return ShaderLib::getBasicShader(String& buffer)
    {
        Return ret = Return::Succeed;
        buffer = _data->buffers[ShaderId::Basic];
        // buffer = _data->buffers[ShaderId::Easy];
        return ret;
    }

    Return ShaderLib::getNodeShader(String& buffer)
    {
        Return ret = Return::Succeed;
        buffer = _data->buffers[ShaderId::Node];
        // buffer = _data->buffers[ShaderId::Easy];
        return ret;
    }

    Return ShaderLib::getShader(ShaderId id, String& buffer)
    {
        Return ret = Return::Succeed;
        auto it = _data->buffers.find(id);
        if (it == _data->buffers.end()) return Return::Failed;
        buffer = (*it).second;
        // buffer = _data->buffers[ShaderId::Easy];
        return ret;
    }

} // namespace EgLab