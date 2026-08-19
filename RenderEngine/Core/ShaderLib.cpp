#include "Core/ShaderLib.hpp"

#include "Common/Return.hpp"
#include "Common/String.hpp"

// ShaderLib
#include "Basic.shader.h"
#include "Common/HashMap.hpp"
#include "Easy.shader.h"
#include "Node.shader.h"

namespace EgLab::Common
{
    template <>
    struct hash<RE::ShaderId>
    {
        size_t operator()(RE::ShaderId x) const
        {
            return static_cast<size_t>(x);
        }
    };

} // namespace EgLab::Common

namespace EgLab::RE
{

    class ShaderLib::Impl
    {
    public:
        Common::HashMap<ShaderId, Common::String> buffers;

        Common::HashMap<ShaderId, Common::SharedPtr<Shader>> shaders;

        Impl()
        {
            buffers.insert(
                {ShaderId::Basic, Common::String(BASIC_SHADER__DATA, BASIC_SHADER__SIZE)});
            buffers.insert({ShaderId::Node, Common::String(NODE_SHADER__DATA, NODE_SHADER__SIZE)});
            buffers.insert({ShaderId::Easy, Common::String(EASY_SHADER__DATA, EASY_SHADER__SIZE)});
        }

        Common::Return getBasicShader(Common::String &buffer)
        {
            Common::Return ret = Common::Return::Succeed;
            buffer = buffers[ShaderId::Basic];
            // buffer = _data->buffers[ShaderId::Easy];
            return ret;
        }

        Common::Return getNodeShader(Common::String &buffer)
        {
            Common::Return ret = Common::Return::Succeed;
            buffer = buffers[ShaderId::Node];
            // buffer = _data->buffers[ShaderId::Easy];
            return ret;
        }

        Common::Return getShader(ShaderId id, Common::String &buffer)
        {
            Common::Return ret = Common::Return::Succeed;
            auto it = buffers.find(id);
            if (it == buffers.end()) return Common::Return::Failed;
            buffer = (*it).second;
            return ret;
        }
    };

    ShaderLib::ShaderLib() : _data(new Impl)
    {
    }

    ShaderLib::~ShaderLib()
    {
        delete _data;
    }

    Common::Return ShaderLib::getBasicShader(Common::SharedPtr<Shader> &shader)
    {
        Common::Return ret = Common::Return::Succeed;
        auto it = _data->shaders.find(ShaderId::Basic);
        if (it == _data->shaders.end())
        {
            Common::String buffer;
            ret = _data->getBasicShader(buffer);
            if (ret != Common::Return::Succeed) return ret;
            it = _data->shaders.insert({ShaderId::Basic, Common::makeShared<Shader>(buffer)});
        }
        shader = (*it).second;
        return ret;
    }

    Common::Return ShaderLib::getNodeShader(Common::SharedPtr<Shader> &shader)
    {
        Common::Return ret = Common::Return::Succeed;
        auto it = _data->shaders.find(ShaderId::Node);
        if (it == _data->shaders.end())
        {
            Common::String buffer;
            ret = _data->getNodeShader(buffer);
            if (ret != Common::Return::Succeed) return ret;
            it = _data->shaders.insert({ShaderId::Node, Common::makeShared<Shader>(buffer)});
        }
        shader = (*it).second;
        return ret;
    }

    Common::Return ShaderLib::getLineShader(Common::SharedPtr<Shader> &shader)
    {
        Common::Return ret = Common::Return::Succeed;
        auto it = _data->shaders.find(ShaderId::Node);
        if (it == _data->shaders.end())
        {
            Common::String buffer;
            ret = _data->getNodeShader(buffer);
            if (ret != Common::Return::Succeed) return ret;
            it = _data->shaders.insert({ShaderId::Node, Common::makeShared<Shader>(buffer)});
        }
        shader = (*it).second;
        return ret;
    }

    Common::Return ShaderLib::getFaceShader(Common::SharedPtr<Shader> &shader)
    {
        Common::Return ret = Common::Return::Succeed;
        auto it = _data->shaders.find(ShaderId::Basic);
        if (it == _data->shaders.end())
        {
            Common::String buffer;
            ret = _data->getBasicShader(buffer);
            if (ret != Common::Return::Succeed) return ret;
            it = _data->shaders.insert({ShaderId::Basic, Common::makeShared<Shader>(buffer)});
        }
        shader = (*it).second;
        return ret;
    }

} // namespace EgLab::RE