#include <Windows.h>
#include "shared/commandline.h"
#include "shared/engine.h"
#include "window.h"

Engine g_engine;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int showCmd)
{
	Window::_programInstance = hInstance;

	RAWINPUTDEVICE rawMouseInput;
	rawMouseInput.usUsagePage = 0x01;
	rawMouseInput.usUsage = 0x02;
	rawMouseInput.dwFlags = 0;
	rawMouseInput.hwndTarget = 0;

	if (::RegisterRawInputDevices(&rawMouseInput, 1, sizeof(rawMouseInput)) == FALSE)
	{
		Debug::Error("Raw mouse input registration error!");
	}

	CommandLine::Init(cmdLine);

	return g_engine.Run();
}
