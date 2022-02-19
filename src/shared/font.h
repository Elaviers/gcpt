#pragma once
#include "asset.h"
#include "rect.h"

class RenderEntry;
class Transform;

/*
	Font
*/

class Font : public Asset
{
protected:
	Font() {}

public:
	virtual ~Font() {}

	static Font* FromText(const String& string);

	virtual float CalculateStringWidth(const char* string, float scaleX, size_t maxChars = 0) const = 0;
	virtual size_t GetPositionOf(float x, float y, const char* string, const Transform& transform, float lineHeight = 0.f, bool round = true) const = 0;

#ifdef BUILD_CLIENT
	virtual void RenderString(RenderEntry&, const char* string, const Transform& transform, float lineHeight = 0.f, const RectF& clip = RectF()) const = 0;
#endif
};
