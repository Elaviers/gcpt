#pragma once

#ifndef NO_ASSERTS
#include "debug.h"
#include "mutex.h"
#include "string.h"

#define AssertM(cond, msg)					\
	if (!(cond)) Debug::FatalError(CSTR(	\
		"Assertion failed!\n", (msg),		\
		"\n-----------------\n"				\
		__FILE__ "@", __LINE__,				\
		":\t" #cond "\n"					\
		"(" __FUNCTION__ ")\n"))

#define Assert(cond) AssertM((cond), #cond)

#define AssertI(a, op, b) AssertM((a op b), CSTR(#a, " (", (a), ") ", #op, ' ', #b, " (", (b), ')'))

#define AssertScopeNotConcurrent()																		\
	static Mutex ___assertmut;																			\
	AssertM(___assertmut.TryLock(), "Concurrency check failed - two or more threads are in scope!") }	\
	MutexHolderNoLock ___assertmutholder(___assertmut);

#else
#define AssertM(expr, msg) (expr)
#define Assert(expr) (expr)
#define AssertI(...)
#define AssertScopeNotconcurrent(...)

#endif
