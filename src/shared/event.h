#pragma once
#include "function.h"
#include "t_list.h"

/*
	Event

	Linked list of function pointers
*/
template<typename ...ARGS>
class Event
{
	List<Function<void, ARGS...>> _list;

public:
	Event() {}
	~Event() {}

	Event& operator+=(const Function<void, ARGS...>& function) 
	{
		//todo- check for duplicates
		/*
		for (const Function<void, ARGS...>& fp : _list)
			if (fp == function)
				return *this;
		*/

		_list.EmplaceBack(function);
		return *this;
	}

	void Clear()
	{
		_list.Clear();
	}

	void operator()(ARGS... args) const 
	{
		for (const Function<void, ARGS...>& fp : _list)
			if (fp)
				fp(std::forward<ARGS>(args)...);
	}
};
