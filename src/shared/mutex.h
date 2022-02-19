#pragma once
#include <Windows.h>
#include <utility>

class Mutex
{
private:
	HANDLE _winHandle;

public:
	Mutex();
	~Mutex();

	Mutex(const Mutex&) = delete;
	Mutex(Mutex&& other) noexcept;

	bool TryLock() noexcept;
	void Lock();
	void Unlock() noexcept;
};

class MutexHolder
{
	Mutex* _mutex;

public:
	MutexHolder(Mutex& mutex) : _mutex(&mutex) { _mutex->Lock(); }
	~MutexHolder() { _mutex->Unlock(); }

	MutexHolder(const MutexHolder&) = delete;
	MutexHolder(MutexHolder&& other) noexcept { _mutex->Unlock(); _mutex = other._mutex; other._mutex = nullptr; }
};

class MutexHolderNoLock
{
	Mutex* _mutex;

public:
	MutexHolderNoLock(Mutex& mutex) : _mutex(&mutex) {}
	~MutexHolderNoLock() { _mutex->Unlock(); }

	MutexHolderNoLock(const MutexHolderNoLock&) = delete;
	MutexHolderNoLock(MutexHolderNoLock&& other) noexcept { _mutex->Unlock(); _mutex = other._mutex; other._mutex = nullptr; }
};

