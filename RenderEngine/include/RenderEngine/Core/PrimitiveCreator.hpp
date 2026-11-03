#pragma once

#include "Common/SharedPtr.hpp"

namespace EgLab::RE
{
    class RenderPrimitive;

    class RenderNode;
    class RenderLine;
    class RenderFace;

    /// Base class that creates and initializes render primitives.
    class PrimitiveCreator
    {
    public:
        /// @brief Destroys the primitive creator.
        virtual ~PrimitiveCreator() = default;

        /**
         * @brief Creates a primitive of the requested type and populates it.
         * @tparam T Render primitive type supported by the updateData overloads.
         * @return The created primitive.
         */
        template <typename T>
        Common::SharedPtr<RenderPrimitive> getPrimitive()
        {
            Common::SharedPtr<T> primitive = Common::makeShared<T>();

            this->updateData(Common::SharedPtr<T>(primitive));

            return primitive;
        }

    protected:
        /// @brief Populates a node primitive with source data.
        virtual void updateData(Common::SharedPtr<RenderNode>) = 0;
        /// @brief Populates a line primitive with source data.
        virtual void updateData(Common::SharedPtr<RenderLine>) = 0;
        /// @brief Populates a face primitive with source data.
        virtual void updateData(Common::SharedPtr<RenderFace>) = 0;
    };
} // namespace EgLab::RE