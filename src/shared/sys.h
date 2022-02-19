#pragma once
#include "cursor.h"

namespace System
{
	ECursor GetCursor();
	void SetCursor(ECursor cursor);

	//Opens console window and binds stdio/iostream to it
	bool OpenConsole();
	void CloseConsole();
}
