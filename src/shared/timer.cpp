#include "timer.h"
#include "debug.h"
#include <Windows.h>


Timer::Timer() : _start(0)
{
	LARGE_INTEGER freq;
	if (::QueryPerformanceFrequency(&freq) == FALSE)
		Debug::FatalError("Please explain to me why you don't have a high frequency performance counter. On a related note, what are you doing on a 20 year old OS?");

	_frequency = freq.QuadPart;
}


Timer::~Timer()
{
}

void Timer::Start()
{
	LARGE_INTEGER value;
	::QueryPerformanceCounter(&value);

	_start = value.QuadPart;
}

float Timer::GetSecondsSinceStart() const
{
	LARGE_INTEGER value;
	::QueryPerformanceCounter(&value);

	return (float)(value.QuadPart - _start) / (float)_frequency;
}

uint64 Timer::GetTicksSinceStart() const
{
	LARGE_INTEGER value;
	::QueryPerformanceCounter(&value);
	return (uint64)value.QuadPart - _start;
}
