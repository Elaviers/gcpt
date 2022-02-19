#pragma once
#include "types.h"

class Timer
{
private:
	uint64 _start;
	uint64 _frequency;

public:
	Timer();
	~Timer();

	void Start();

	uint64 GetTicksPerSecond() const { return _frequency; }
	uint64 GetTicksSinceStart() const;
	float GetSecondsSinceStart() const;
};
