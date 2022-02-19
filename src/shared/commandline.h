#pragma once

namespace CommandLine
{
	void Init(const char* commandLine);

	const char* FindParam(const char* param);
	int FindParamValue(const char* param, char* valBuf, int valBufSz);
}
