/**
 * @file Global.h
 * @author Moussa-Li
 * @brief the global assets common storage place
 * @date 2025-10-25
 */
 #pragma once

#include "Common/CommonAPI.hpp"
#include "Common/Singleton.hpp"

#define RegisterAssets(NAME, RegisterFunc) \
    if constexpr (assetId == NAME) { \
        return Assets::instance().RegisterFunc(buffer); \
    }


namespace EgLab
{
    enum class AssetId {
        Basic
    };

    class String;
    enum class Return;

    class CommonAPI Assets : public Singleton<Assets>
    {
    public:
        Return getBasicShader(String &buffer);

        Return getLightShader(String &buffer);

    private:
        Assets();
        ~Assets();
        friend class Singleton<Assets>;

        class Impl;
        Impl *_data;

    };

    template<AssetId assetId>
    Return getAssets(String& buffer){

        RegisterAssets(AssetId::Basic, getBasicShader);
        

    }

}