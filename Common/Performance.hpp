#pragma once

#include <chrono>
#include <cstdint>

namespace EgLab::Common
{
	class Performance
	{
	public:
		using SteadyClock = std::chrono::steady_clock;
		using SystemClock = std::chrono::system_clock;
		using TimePoint = SystemClock::time_point;

		void start() noexcept
		{
			_startTime = SystemClock::now();
			_startTick = SteadyClock::now();
			_endTime = TimePoint();
			_endTick = SteadyClock::time_point();
			_running = true;
		}

		void stop() noexcept
		{
			if (!_running) return;

			_endTime = SystemClock::now();
			_endTick = SteadyClock::now();
			_running = false;
		}

		bool isRunning() const noexcept
		{
			return _running;
		}

		TimePoint getStartTime() const noexcept
		{
			return _startTime;
		}

		TimePoint getEndTime() const noexcept
		{
			return _endTime;
		}

		std::int64_t getStartTimestampMs() const noexcept
		{
			return timestampMs(_startTime);
		}

		std::int64_t getEndTimestampMs() const noexcept
		{
			return timestampMs(_endTime);
		}

		double getElapsedMilliseconds() const noexcept
		{
			const auto endTick = _running ? SteadyClock::now() : _endTick;
			if (_startTick == SteadyClock::time_point() ||
				endTick == SteadyClock::time_point())
			{
				return 0.0;
			}

			return std::chrono::duration<double, std::milli>(endTick - _startTick).count();
		}

	private:
		static std::int64_t timestampMs(TimePoint time) noexcept
		{
			if (time == TimePoint()) return 0;

			return std::chrono::duration_cast<std::chrono::milliseconds>(
					   time.time_since_epoch())
				.count();
		}

	private:
		TimePoint _startTime{};
		TimePoint _endTime{};
		SteadyClock::time_point _startTick{};
		SteadyClock::time_point _endTick{};
		bool _running{false};
	};
} // namespace EgLab::Common
