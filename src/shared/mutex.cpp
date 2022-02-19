#include "mutex.h"
#include "assert.h"

Mutex::Mutex() : _winHandle(0)
{
}

Mutex::Mutex(Mutex&& other) noexcept : _winHandle(other._winHandle)
{
	other._winHandle = 0;
}

Mutex::~Mutex()
{
	if (_winHandle)
		::CloseHandle(_winHandle);
}

bool Mutex::TryLock() noexcept
{
	if (!_winHandle)
	{
		_winHandle = ::CreateMutex(nullptr, FALSE, nullptr);
		Assert(_winHandle);
	}

	const DWORD result = ::WaitForSingleObject(_winHandle, 0);
	Assert(result != WAIT_ABANDONED); //A thread terminated with a locked mutex in it
	return result == WAIT_OBJECT_0;
}

void Mutex::Lock()
{
	if (!_winHandle)
	{
		_winHandle = ::CreateMutex(nullptr, FALSE, nullptr);
		Assert(_winHandle);
	}

	const DWORD result = ::WaitForSingleObject(_winHandle, INFINITE);
	Assert(result != WAIT_ABANDONED); //A thread terminated with a locked mutex in it
}

void Mutex::Unlock() noexcept
{
	if (_winHandle)
		::ReleaseMutex(_winHandle);
}
