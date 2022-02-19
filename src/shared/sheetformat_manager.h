#pragma once
#include "asset_manager.h"
#include "sheetformat.h"

class SheetFormatManager : public AssetManager<SheetFormat>
{
	virtual SheetFormat* _CreateResource(const Array<byte>& data, const String& name, const String& extension);

};
