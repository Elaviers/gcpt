#include "font_manager.h"
#include "font.h"
#include "font_bitmap.h"
#include "font_ttf.h"

Font* FontManager::_CreateResource(const Array<byte>& data, const String& name, const String& extension)
{
	Font* font = Font::FromText(String(data.begin(), data.GetSize()));

	if (font == nullptr)
		Debug::Error(CSTR("Unknown type for font \"", name, '\"'));

	return font;
}
