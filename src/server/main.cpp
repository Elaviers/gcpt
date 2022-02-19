#include "shared/commandline.h"
#include "shared/engine.h"

Engine g_engine;

int main(int argsc, char* argsv[])
{
	{
		char fullCommandString[8192];
		int offset = 0;
		
		for (int i = 1; i < argsc; ++i)
		{
			const char* fmtstr = "%s ";

			for (const char* c = argsv[i]; *c; ++c)
			{
				if (*c == ' ' || *c == '\t')
				{
					fmtstr = "\"%s\" ";
					break;
				}
			}

			offset += StringUtils::snprintf(fullCommandString + offset, sizeof(fullCommandString) - 1 - offset, fmtstr, argsv[i]);
		}

		CommandLine::Init(fullCommandString);
	}

	return g_engine.Run();
}
