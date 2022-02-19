#include "material.h"
#include "material_grid.h"
#include "material_surface.h"
#include "material_sprite.h"
#include "material_spritesheet.h"
#include "shared/macro_utils.h"

Material* Material::FromText(const String& text)
{
	if (text.GetLength())
	{
		size_t splitIndex = text.IndexOfAny("\r\n");
		String firstLine = text.SubString(0, splitIndex).ToLower();
		String relevantData = text.SubString(splitIndex + 1);

		if (firstLine == "surface")
			return Asset::FromText<MaterialSurface>(relevantData);
		else if (firstLine == "grid")
			return Asset::FromText<MaterialGrid>(relevantData);
		else if (firstLine == "sprite")
			return Asset::FromText<MaterialSprite>(relevantData);
		else if (firstLine == "spritesheet")
			return Asset::FromText<MaterialSpritesheet>(relevantData);
	}

	return nullptr;
}
