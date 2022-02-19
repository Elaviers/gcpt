#include "font.h"
#include "font_bitmap.h"
#include "font_ttf.h"
#include "font_manager.h"

Font* Font::FromText(const String& text)
{
	if (text.GetLength())
	{
		size_t splitIndex = text.IndexOfAny("\r\n");
		String firstLine = text.SubString(0, splitIndex).ToLower();
		String relevantData = text.SubString(splitIndex + 1);

		if (firstLine == "bitmap")
			return Asset::FromText<FontTexture>(relevantData);
		else if (firstLine == "ttf")
			return Asset::FromText<FontTTF>(relevantData);
	}

	return nullptr;
}
