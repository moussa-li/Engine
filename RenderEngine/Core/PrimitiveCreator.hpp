#pragma once

#include "Common/SharedPtr.hpp"

namespace EgLab::RE
{
    class RenderPrimitive;

    class RenderNode;
    class RenderLine;
    class RenderFace;

    class PrimitiveCreator
    {
    public:
        virtual ~PrimitiveCreator() = default;

        template <typename T>
        Common::SharedPtr<RenderPrimitive> getPrimitive()
        {
            Common::SharedPtr<T> primitive = Common::makeShared<T>();

            this->updateData(Common::SharedPtr<T>(primitive));

            primitive->setup();

            return primitive;
        }

    protected:
        virtual void updateData(Common::SharedPtr<RenderNode>) = 0;
        virtual void updateData(Common::SharedPtr<RenderLine>) = 0;
        virtual void updateData(Common::SharedPtr<RenderFace>) = 0;
    };
} // namespace EgLab::RE