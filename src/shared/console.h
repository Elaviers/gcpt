#pragma once
#include <vadefs.h>

void Console_Clear();
void Console_Msg(const char* fmt, ...);
void Console_Warning(const char* fmt, ...);
void Console_VMsg(const char* fmt, va_list vargs);

void Console_ImGui();
