#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include "font.h"
#include "t_hashmap.h"
#include "t_vec2.h"

#ifdef BUILD_CLIENT
#include "client/gl.h"
#endif

class FontTTF : public Font
{
private:

public:
	struct TTFGlyph
	{
#ifdef BUILD_CLIENT
		GLuint texID = 0;
#endif

		Vector2 size;
		Vector2 bearing;

		FT_Pos advance = 0;
	};

private:
	FT_Face _face;

	int _descender;

	Hashmap<char, TTFGlyph> _charMap;

#ifdef BUILD_CLIENT
	GLuint _vao, _vbo;
#endif

	int _size;

	void _CMD_LoadFont(const Array<String>& args);

public:
	FontTTF();
	virtual ~FontTTF() {}

	virtual const PropertyCollection& GetProperties() override;

	virtual float CalculateStringWidth(const char* string, float scaleX, size_t maxChars = 0) const override;
	virtual size_t GetPositionOf(float x, float y, const char* string, const Transform& transform, float lineHeight = 0.f, bool round = true) const override;

#ifdef BUILD_CLIENT
	virtual void RenderString(RenderEntry&, const char *string, const Transform &transform, float lineHeight = 0.f, const RectF& clip = RectF()) const override;
#endif

	constexpr const int GetPointSize() const noexcept { return _size; }
	constexpr const Hashmap<char, TTFGlyph>& GetGlyphMap() const noexcept { return _charMap; }
};
