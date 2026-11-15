#pragma once

#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Work/RenderEvent.hpp"

namespace EgLab::Platform
{
    class RenderDataManager;

    /// Strategy for handling one kind of RenderEvent.
    ///
    /// The handler lifecycle follows the RenderWork event pipeline:
    /// 1. prepare()  - runs on the event-receiving thread, without the render data
    ///                 mutex, and performs lock-free preprocessing such as mesh
    ///                 deserialization or render packet creation.
    /// 2. record()   - runs on the event-receiving thread while the render data mutex
    ///                 is held, and records the requested state.
    /// 3. apply()    - runs on the render thread while the render data mutex is held,
    ///                 and applies the request to the scene.
    class IRenderEventHandler
    {
    public:
        virtual ~IRenderEventHandler() = default;

        virtual Common::Return prepare(RenderEvent& event)
        {
            (void)event;
            return Common::Return::Succeed;
        }

        virtual Common::Return record(const RenderEvent& event)
        {
            (void)event;
            return Common::Return::Succeed;
        }

        virtual Common::Return apply(const RenderEvent& event)
        {
            (void)event;
            return Common::Return::Succeed;
        }
    };

    /// Creates the IRenderEventHandler responsible for a RenderEventType.
    class RenderEventHandlerFactory
    {
    public:
        explicit RenderEventHandlerFactory(RenderDataManager& data);

        Common::SharedPtr<IRenderEventHandler> create(RenderEventType type) const;

    private:
        RenderDataManager& _data;
    };
} // namespace EgLab::Platform
