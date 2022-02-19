#pragma once
#include "font.h"
#include "glyph.h"
#include "function.h"
#include "t_hashmap.h"
#include "shared_ptr.h"
#include "string.h"
#include "transform.h"

#ifdef BUILD_CLIENT
#include "client/texture.h"
#endif

class FontTexture : public Font
{
#ifdef BUILD_CLIENT
	SharedPointer<const Texture> _texture;
#endif
	
	Hashmap<byte, Glyph> _charMap;

	int _size = 0;
	int _rowH = 0;
	int _yOffset = 0;

	void _CMD_texture(const Array<String>& args);
	void _CMD_region(const Array<String>& args);

protected:
	virtual void ReadText(const String&) override;

public:
	FontTexture() {}

	virtual ~FontTexture() {}

	virtual const PropertyCollection& GetProperties() override;

	virtual float CalculateStringWidth(const char* string, float scaleX, size_t maxChars = 0) const override;
	virtual size_t GetPositionOf(float x, float y, const char* string, const Transform& transform, float lineHeight = 0.f, bool round = true) const override;

#ifdef BUILD_CLIENT
	virtual void RenderString(RenderEntry&, const char* string, const Transform & transform, float lineHeight = 0.f, const RectF& clip = RectF()) const override;

	void RenderString(RenderEntry& e, const char* string, const Transform& transform) const { return RenderString(e, string, transform, -transform.GetScale().y); }

	void BindTexture() const { if (_texture) _texture->Bind(0); }
#endif
};
