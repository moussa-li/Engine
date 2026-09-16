#pragma once

#include "Command/CommandParam.hpp"
#include "Common/Log.hpp"
#include "Common/Performance.hpp"
#include "Common/Return.hpp"
#include "Common/Utils.hpp"

namespace EgLab::Platform
{
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
                endTransaction();
                _performance.stop();
                LOG(INFO) << "Finished executing command: " << getCmdName()
                          << ", end_ms=" << _performance.getEndTimestampMs()
                          << ", elapsed_ms=" << _performance.getElapsedMilliseconds();
                return ret;
            }
            catch (...)
            {
                _performance.stop();
                LOG(ERROR) << "Command failed: " << getCmdName()
                           << ", end_ms=" << _performance.getEndTimestampMs()
                           << ", elapsed_ms=" << _performance.getElapsedMilliseconds();
                throw;
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
            return Common::Return::Succeed;
        }

    protected:
        T _params;
        Common::Performance _performance;
    };

} // namespace EgLab::Platform