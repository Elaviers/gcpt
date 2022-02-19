#include "string_utils.h"
#include "assert.h"
#include "maths.h"
#include "console.h"

uint StringUtils::strncpy(char* dst, int dstSz, const char* src)
{
	Assert(src);
	Assert(dst);
	Assert(dstSz);

	const char* const start = dst;
	const char* const end = src + dstSz;

	while (src < end && *src)
		*(dst++) = *(src++);

	*dst = '\0';
	return (int)(dst - start);
}

uint StringUtils::vsnprintf(char* dst, int dstSz, const char* fmt, va_list args)
{
	int res = std::vsnprintf(dst, dstSz, fmt, args);

	if (res < 0)
	{
		Console_Warning("malformed vsnprintf - fmt string -- %s\n", fmt);
		return 0;
	}

	return Maths::Min(res, dstSz - 1);
}

uint StringUtils::snprintf(char* dst, int dstSz, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	const uint res = StringUtils::vsnprintf(dst, dstSz, fmt, args);
	va_end(args);

	return res;
}
