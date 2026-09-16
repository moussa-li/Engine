#pragma once

#include "Common/String.hpp"

namespace EgLab::Platform
{
    class IPanel
    {
    public:
        IPanel();
        virtual ~IPanel();

        virtual Common::String getPanelName() = 0;

        inline bool isShow() const
        {
            return _show;
        }

        void show();
        void close();

        virtual void render() = 0;

    protected:
        virtual void abortShow() {};
        virtual void abortClose() {};

    private:
        bool _show{false};
    };
} // namespace EgLab::Platform