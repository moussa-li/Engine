#include <gtest/gtest.h>

#include <atomic>

#include "Command/CommandBus.hpp"
#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    namespace
    {
        class CountingCommand : public ICommand
        {
        public:
            Common::Return exec() override
            {
                ++_executionCount;
                return Common::Return::Succeed;
            }

            static void reset()
            {
                _executionCount = 0;
            }

            static int count()
            {
                return _executionCount.load();
            }

        private:
            static std::atomic<int> _executionCount;
        };

        std::atomic<int> CountingCommand::_executionCount{0};
    } // namespace
} // namespace EgLab::Platform

TEST(CommandBus, StopDiscardsPendingCommands)
{
    using namespace EgLab;

    Platform::CommandBus bus(0);
    Platform::CountingCommand::reset();
    auto command = Common::makeShared<Platform::CountingCommand>();

    EXPECT_EQ(bus.dispatch(command), Common::Return::Succeed);
    bus.stop(true);

    EXPECT_EQ(Platform::CountingCommand::count(), 0);
    EXPECT_EQ(bus.dispatch(command), Common::Return::Failed);
}
