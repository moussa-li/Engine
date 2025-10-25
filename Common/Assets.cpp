#include "Common/Assets.hpp"
#include "Common/String.hpp"
#include "Common/Return.hpp"


// Assets
#include "Basic.shader.h"

namespace EgLab
{
    class Assets::Impl
    {
    public:
        String BasicBuffer;
    };

    Assets::Assets() 
        : _data(new Impl)
    {
        _data->BasicBuffer = String(BASIC_SHADER__DATA,BASIC_SHADER__SIZE);

    }

    Assets::~Assets()
    {
        delete _data;
    }

    Return Assets::getBasicShader(String& buffer) {
        Return ret = Return::Succeed;
        buffer = _data->BasicBuffer;
        return ret;
    }


}