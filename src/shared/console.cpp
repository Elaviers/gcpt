#include "console.h"
#include "convar.h"
#include "engine.h"
#include "assert.h"
#include "t_memory.h"
#include "maths.h"
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <mutex>

#if _WIN32
#include <Windows.h>
#endif

constexpr const int CONSOLE_MAX_MSG_LEN = 2048;
constexpr const int CONSOLE_MAX_BUFSIZE = 1024 * 1024;
constexpr const int CONSOLE_HISTORY_SIZE = 1024;

static AllocatedMemory<char> s_consoleBuffer;
static uint64 s_consoleBegin = 0;
static uint64 s_consoleLength = 0;

static std::mutex s_consoleMutex;
static char s_consoleMsgBuffer[CONSOLE_MAX_MSG_LEN];

static char* s_consoleHistory[CONSOLE_HISTORY_SIZE];
static int s_consoleHistoryBegin = 0;
static int s_consoleHistoryEntryCount = 0;
static int s_consoleHistoryCurrentIdx = -1; // <0 = none

void Console_AddToHistory(const char* str)
{
	int length = StringUtils::Length(str);
	char* memory = new char[length + 1];
	Utilities::CopyMemory(str, memory, length);
	memory[length] = '\0';

	if (s_consoleHistoryEntryCount < CONSOLE_HISTORY_SIZE)
		s_consoleHistory[s_consoleHistoryEntryCount++] = memory;
	else
	{
		delete[] s_consoleHistory[s_consoleHistoryBegin];
		s_consoleHistory[s_consoleHistoryBegin] = memory;
		s_consoleHistoryBegin = (s_consoleHistoryBegin + 1) % CONSOLE_HISTORY_SIZE;
	}
}

int Console_LastHistoryIdx()
{
	if (s_consoleHistoryEntryCount < CONSOLE_HISTORY_SIZE)
		return s_consoleHistoryEntryCount - 1;

	return s_consoleBegin - 1;
}

void Console_Clear()
{
	s_consoleBuffer.Shrink(0);
	s_consoleBegin = s_consoleLength = 0;
}

void Console_VMsg(const char* fmt, va_list vargs)
{
	s_consoleMutex.lock();

	int length = 0;
	if (s_consoleLength == 0 || s_consoleBuffer.GetBase()[(s_consoleBegin + s_consoleLength - 1) % s_consoleBuffer.GetSize()] == '\n')
		length = snprintf(s_consoleMsgBuffer, CONSOLE_MAX_MSG_LEN, "[%1.3f] ", g_engine.GetTimeSeconds());

	length += vsnprintf(s_consoleMsgBuffer + length, CONSOLE_MAX_MSG_LEN - length, fmt, vargs);
	Assert(length >= 0);

#if _WIN32
	::OutputDebugStringA(s_consoleMsgBuffer);
#endif

	const size_t insertionIdx = s_consoleBuffer.GetSize() > 0 ? ((s_consoleBegin + s_consoleLength) % s_consoleBuffer.GetSize()) : 0;
	const uint64 newBufferLength = s_consoleLength + length + 1;

	if (newBufferLength > s_consoleBuffer.GetSize() && newBufferLength <= CONSOLE_MAX_BUFSIZE && s_consoleBuffer.GetSize() < CONSOLE_MAX_BUFSIZE)
	{
		if (s_consoleBegin + s_consoleLength > s_consoleBuffer.GetSize())
		{
			const size_t gapSize = newBufferLength - s_consoleBuffer.GetSize();
			s_consoleBuffer.MakeGap(s_consoleLength, insertionIdx, gapSize);
		}
		else
			s_consoleBuffer.Grow(s_consoleLength, newBufferLength);
	}

	char* const out = s_consoleBuffer.GetBase() + insertionIdx;
	if (newBufferLength <= s_consoleBuffer.GetSize())
	{
		memcpy(out, s_consoleMsgBuffer, length);
		out[length] = '\0';
		s_consoleLength += length;
	}
	else
	{
		const uint64 tSize = length + 1;
		const uint64 p1Size = Maths::Min(tSize, s_consoleBuffer.GetSize() - insertionIdx);
		const uint64 p2Size = (p1Size != tSize) ? (tSize - p1Size) : 0;

		memcpy(out, s_consoleMsgBuffer, p1Size);

		if (p2Size)
			memcpy(s_consoleBuffer.GetBase(), s_consoleMsgBuffer + p1Size, p2Size);

		s_consoleBegin = (insertionIdx + tSize) % s_consoleBuffer.GetSize();
		s_consoleLength = s_consoleBuffer.GetSize() - 1;
	}

	s_consoleMutex.unlock();
}

void Console_Msg(const char* fmt, ...)
{
	va_list vargs;

	va_start(vargs, fmt);
	Console_VMsg(fmt, vargs);
	va_end(vargs);
}

void Console_Warning(const char* fmt, ...)
{
	va_list vargs;

	va_start(vargs, fmt);
	Console_VMsg(fmt, vargs);
	va_end(vargs);
}

#include "imgui.h"

static char s_consoleInput[512];

void Console_ImGui()
{
	ImGui::Begin("Console");
	ImGui::SetWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	{
		static ImGuiTextFilter filter;
		filter.Draw("##filter", -FLT_MIN);

		ImGui::BeginChild("ConsoleText", ImVec2(-FLT_MIN, -24), true, 0);
		
		const float prevScroll = ImGui::GetScrollY();
		const float prevScrollMax = ImGui::GetScrollMaxY();

		if (s_consoleLength)
		{
			const char* const begin = s_consoleBuffer.GetBase() + s_consoleBegin;
			const char* const end = s_consoleBuffer.GetBase() + s_consoleBuffer.GetSize();
			const char* text = begin;
			const char* next = begin;

			bool passed = true;
			while (*next)
			{
				if (next >= end)
				{
					passed = filter.PassFilter(text, next);

					if (passed)
					{
						ImGui::TextUnformatted(text, next);
						ImGui::SameLine(0, 0);
					}

					text = next = s_consoleBuffer.GetBase();
					continue;
				}

				if (*next == '\n' || *next == '\r')
				{
					if (passed)
					{
						if (filter.PassFilter(text, next))
							ImGui::TextUnformatted(text, next);
					}

					++next;
					text = next;
					passed = true;
					continue;
				}

				++next;
			}

			if (next > text)
				ImGui::TextUnformatted(text, next);

			if (prevScroll == prevScrollMax)
				ImGui::SetScrollHereY(1.f);
		}

		ImGui::EndChild();
	}

	auto inputTextCallback = [](ImGuiInputTextCallbackData* data) -> int
	{
		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackCompletion:
		{
			//todo
		}
		break;

		case ImGuiInputTextFlags_CallbackHistory:
		{
			const int prev = s_consoleHistoryCurrentIdx;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (s_consoleHistoryCurrentIdx < 0)
					s_consoleHistoryCurrentIdx = Console_LastHistoryIdx();
				else if (s_consoleHistoryCurrentIdx != s_consoleHistoryBegin)
					s_consoleHistoryCurrentIdx = (s_consoleHistoryCurrentIdx - 1) % CONSOLE_HISTORY_SIZE;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (s_consoleHistoryCurrentIdx >= 0)
				{
					if (s_consoleHistoryCurrentIdx == Console_LastHistoryIdx())
						s_consoleHistoryCurrentIdx = -1;
					else
						s_consoleHistoryCurrentIdx = (s_consoleHistoryCurrentIdx + 1) % CONSOLE_HISTORY_SIZE;
				}
			}

			if (s_consoleHistoryCurrentIdx != prev && s_consoleHistoryCurrentIdx < s_consoleHistoryEntryCount)
			{
				const char* str = s_consoleHistoryCurrentIdx < 0 ? "" : s_consoleHistory[s_consoleHistoryCurrentIdx];
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, str);
			}
			else
				s_consoleHistoryCurrentIdx = prev;
		}
		break;
		}

		return 0;
	};

	ImGui::SetNextItemWidth(-FLT_MIN);
	const ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
	if (ImGui::InputTextWithHint("##input", "console input", s_consoleInput, sizeof(s_consoleInput), flags, inputTextCallback))
	{
		//Console_Msg("%s\n", s_consoleInput);
		Con_ExecuteCommand(s_consoleInput);

		Console_AddToHistory(s_consoleInput);
		s_consoleHistoryCurrentIdx = -1;
		s_consoleInput[0] = '\0';

		ImGui::SetKeyboardFocusHere(-1); //refocus prev widget (input box)
	}

	ImGui::End();
}

ConCommand cc_echo("echo", "Console message", [](const ConArgs& args) {
	
	if (args.GetArgCount() > 1)
	{
		Console_Msg("%s\n", args.GetArg(1).begin());
	}

});

ConCommand cc_clear("clear", "Clear console window", [](const ConArgs&) {
	Console_Clear();
	
});
