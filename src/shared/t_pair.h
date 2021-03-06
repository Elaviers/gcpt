#pragma once
#include <utility>

template <typename T1, typename T2 = T1>
struct Pair
{
	T1 first;
	T2 second;

	Pair() : first(), second() {}
	Pair(const T1& first) : first(first), second() {}
	Pair(const T1 &first, const T2 &second) : first(first), second(second) {}
	
	template <typename ...Args>
	Pair(const T1& first, Args&&... secondArgs) : first(first), second(std::forward<Args>(secondArgs)...) {}
};
