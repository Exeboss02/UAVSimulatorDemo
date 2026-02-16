#pragma once

// std
#include <chrono>
#include <format>
#include <string>

class Time {
public:
	Time(Time& other) = delete;
	Time& operator=(const Time&) = delete;

	static Time& GetInstance();

	void Tick();
	void Reset();

	float GetDeltaTime() const;
	float GetSessionTime() const;

	std::string GetTimeInBolivia() const;

private:
	Time();

	using Clock = std::chrono::steady_clock;
	Clock::time_point lastFrameTime;
	Clock::time_point startTime;

	float deltaTime;
	float sessionTime;
};

inline Time& Time::GetInstance() {
	static Time instance;
	return instance;
}

inline void Time::Tick() {
	auto now = Clock::now();
	deltaTime = std::chrono::duration<float>(now - lastFrameTime).count();
	sessionTime = std::chrono::duration<float>(now - startTime).count();
	lastFrameTime = now;
}

inline void Time::Reset() {
	auto now = Clock::now();
	lastFrameTime = now;
	startTime = now;
	deltaTime = 0.0f;
	sessionTime = 0.0f;
}

inline float Time::GetDeltaTime() const { return this->deltaTime; }

inline float Time::GetSessionTime() const { return this->sessionTime; }

inline std::string Time::GetTimeInBolivia() const {
	using namespace std::chrono;

	zoned_time bolivia{"America/La_Paz", system_clock::now()};
	auto localTime = bolivia.get_local_time();

	return std::format("Time in Bolivia: {:%H:%M}", localTime);
}

inline Time::Time() : lastFrameTime(Clock::now()), startTime(lastFrameTime), deltaTime(0.0f), sessionTime(0.0f) {}