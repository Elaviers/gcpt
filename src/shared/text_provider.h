#pragma once
#include "t_hashmap.h"
#include "text.h"

class TextProvider
{
private:
	Hashmap<String, String> _strings;

public:
	TextProvider() {}
	virtual ~TextProvider() {}

	constexpr bool IsEmpty() const { return _strings.GetSize() == 0; }

	Text Get(const String& key) const;

	void Set(const String& key, const String& value);
	
	/*
		Set
		Adds key/value pairs from one string

		Format:
		KEY=VALUE
		KEY2=VALUE2
		KEY3=VALUE3

		The string is only split on newlines and equals signs
		'\n' may be used to denote newlines
		keys cannot contain a '=' character, while values may do so
	*/
	void Set(const String& data);
};
