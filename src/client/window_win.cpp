#include "window_win.h"

#ifdef WIN32

void Window_Win32::Create(LPCTSTR className, LPCTSTR title, DWORD flags, HWND parent, LPVOID param)
{
	if (param == nullptr)
		param = this;

	if (_hwnd)
		::DestroyWindow(_hwnd);

	_hwnd = ::CreateWindow(className, title, flags, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, parent, NULL, ::GetModuleHandle(NULL), param);

	_hdc = GetDC(_hwnd);
	WindowFunctions_Win32::SetDefaultPixelFormat(_hdc);
}

void Window_Win32::CreateEx(DWORD exStyle, LPCTSTR className, LPCTSTR title, DWORD style, HWND parent, LPVOID param)
{
	if (param == nullptr)
		param = this;

	if (_hwnd)
		::DestroyWindow(_hwnd);

	_hwnd = ::CreateWindowEx(exStyle, className, title, style, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, parent, NULL, ::GetModuleHandle(NULL), param);

	_hdc = GetDC(_hwnd);
	WindowFunctions_Win32::SetDefaultPixelFormat(_hdc);
}

#endif
