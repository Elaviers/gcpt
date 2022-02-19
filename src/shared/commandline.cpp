#include "commandline.h"
#include "assert.h"
#include "string_utils.h"
#include <cstring>

static bool s_cmdline_initialised = false;

constexpr const int COMMANDLINE_MAX_SZ = 8192;
static char s_cmdLine[COMMANDLINE_MAX_SZ];

void CommandLine::Init(const char* commandLine)
{
	StringUtils::strncpy(s_cmdLine, sizeof(s_cmdLine), commandLine);

	s_cmdline_initialised = true;
}

const char* CommandLine::FindParam(const char* param)
{
	Assert(s_cmdline_initialised);
	Assert(param);

	const size_t paramLen = StringUtils::Length(param);

	const char* found = s_cmdLine;
	while (found = std::strstr(found, param))
	{
		const char term = found[paramLen];
		if (term == '\0' || term == ' ' || term == '\t')
			break;
	}

	return found;
}

int CommandLine::FindParamValue(const char* param, char* valBuf, int valBufSz)
{
	Assert(s_cmdline_initialised);
	Assert(param);
	Assert(valBuf);
	Assert(valBufSz);

	valBuf[0] = '\0';

	const char* found = FindParam(param);
	if (found)
	{
		found += strlen(param);

		if (*found == '\0')
			return 0;

		while (*found == ' ' || *found == '\t')
			if (*++found == '\0')
				return 0;

		const char* rd = found;
		char* wr = valBuf;
		const char* const end = valBuf + valBufSz - 1;

		bool escaped = false;
		bool quoted = false;
		while (wr < end)
		{
			const char ch = *rd;

			if (ch == '\\')
			{
				escaped = true;
				++rd;
				continue;
			}

			if (escaped)
			{
				if (ch != '\"')
				{
					++rd;
					continue;
				}
			}
			else
			{
				if (ch == '\"')
				{
					quoted = !quoted;
					++rd;
					continue;
				}
			}

			if (!quoted && (ch == ' ' || ch == '\t'))
			{
				break;
			}

			*wr++ = ch;
			++rd;
		}

		*wr = '\0';
		return wr - valBuf;
	}

	return 0;
}
