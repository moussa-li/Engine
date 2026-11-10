#pragma once

#include <cstdio>

#include "Command/CommandParam.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/Exception.hpp"
#include "Common/Log.hpp"
#include "Common/MessageCenter.hpp"
#include "Common/Performance.hpp"
#include "Common/Return.hpp"
#include "Common/Utils.hpp"

namespace EgLab::Platform
{
    Common::Return commitPendingRenderUpdates();

    class ICommand
    {
    public:
        virtual ~ICommand() = default;

        virtual Common::Return exec() = 0;

        virtual Common::String getCmdName() const
        {
            return Common::String("ICommand");
        }
    };

    template <typename T>
    class CommandBase : public ICommand
    {
    public:
        explicit CommandBase(const T& params) : _params(params)
        {
        }

        explicit CommandBase(T&& params) : _params(Common::move(params))
        {
        }

        virtual Common::String getCmdName() const override
        {
            return Common::String("CommandBase<") + Common::String(typeid(T).name()) +
                   Common::String(">");
        }

        const T& getParams() const
        {
            return _params;
        }

        const Common::Performance& getPerformance() const
        {
            return _performance;
        }

        virtual Common::Return exec() override
        {
            _performance.start();
            LOG(INFO) << "Executing command: " << getCmdName()
                      << ", start_ms=" << _performance.getStartTimestampMs();

            try
            {
                beginTransaction();
                Common::Return ret = execImpl();
                const Common::Return endResult = endTransaction();
                if (ret == Common::Return::Succeed && endResult != Common::Return::Succeed)
                {
                    ret = endResult;
                }
                _performance.stop();
                LOG(INFO) << "Finished executing command: " << getCmdName()
                          << ", end_ms=" << _performance.getEndTimestampMs()
                          << ", elapsed_ms=" << _performance.getElapsedMilliseconds();

                // 每条命令执行结束后发送一条消息，附上运行耗时。
                char commandMessage[192];
                std::snprintf(commandMessage, sizeof(commandMessage),
                              "Command %s finished, elapsed_ms=%.3f", getCmdName().c_str(),
                              _performance.getElapsedMilliseconds());
                Common::MessageCenter::instance().emit(ret == Common::Return::Succeed
                                                           ? Common::MessageLevel::Info
                                                           : Common::MessageLevel::Warning,
                                                       commandMessage);
                return ret;
            }
            catch (Common::Exception& e)
            {
                _performance.stop();
                LOG(ERROR) << e.what();
                LOG(ERROR) << "Command failed: " << getCmdName()
                           << ", end_ms=" << _performance.getEndTimestampMs()
                           << ", elapsed_ms=" << _performance.getElapsedMilliseconds();

                // 命令执行异常时同样发送一条带耗时的错误消息。
                char commandMessage[192];
                std::snprintf(commandMessage, sizeof(commandMessage),
                              "Command %s failed, elapsed_ms=%.3f", getCmdName().c_str(),
                              _performance.getElapsedMilliseconds());
                Common::MessageCenter::instance().emit(Common::MessageLevel::Error, commandMessage);
                return Common::Return::Failed;
            }
        }

    protected:
        virtual Common::Return execImpl() = 0;

        virtual Common::Return beginTransaction()
        {
            return Common::Return::Succeed;
        }

        virtual Common::Return endTransaction()
        {
            return commitPendingRenderUpdates();
        }

    protected:
        T _params;
        Common::Performance _performance;
    };

} // namespace EgLab::Platform