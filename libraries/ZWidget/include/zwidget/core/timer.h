#pragma once

#include <functional>

class Widget;

class Timer
{
public:
	Timer(Widget* owner);
	~Timer();

	void Start(int timeoutMilliseconds, bool repeat = true);
	void Stop();

	std::function<void()> FuncExpired;
	
	void SetTimerId(void* id) { TimerId = id; }
	void* GetTimerId() const { return TimerId; }

private:
	Widget* OwnerObj = nullptr;
	Timer* PrevTimerObj = nullptr;
	Timer* NextTimerObj = nullptr;

	    void* TimerId = nullptr;
	};
