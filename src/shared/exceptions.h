#pragma once
#include <exception>

class ExcInvalidParam : public std::exception
{
public:
	ExcInvalidParam(const char* msg = "Invalid parameter") : std::exception(msg) {}
};

class KeyDoesNotExist : public std::exception
{
public:
	virtual char const* what() const override
	{
		return "Key does not exist";
	}
};
