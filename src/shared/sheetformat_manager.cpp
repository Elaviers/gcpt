#include "sheetformat_manager.h"
#include "debug.h"

SheetFormat* SheetFormatManager::_CreateResource(const Array<byte>& data, const String& name, const String& extension)
{
	SheetFormat* sheetFormat = new SheetFormat();
	if (!sheetFormat->Parse(String(data.begin(), data.GetSize())))
		Debug::Error(CSTR("Error loading SheetFormat \"", name, "\""));

	return sheetFormat;
}
