#pragma once
#include "ui_element.h"
#include "ui_colour.h"
#include "shared/shared_ptr.h"
#include "shared/text.h"
#include "shared/transform.h"
#include "shared/t_vec2.h"
#include "shared/font.h"

enum class ETextAlignment { CENTRE, LEFT, RIGHT };

class UILabel : public UIElement
{
	SharedPointer<const Font> _font;
	Text _text;

	UIColour _colour;
	UIColour _shadowColour;

	Transform _transform;
	Transform _shadowTransform;
	RectF _clip;
	RectF _shadowClip;

	Vector2 _offset;
	Vector2 _shadowOffset;

	ETextAlignment _alignment;
	float _margin;

	float _textSize;

	void _UpdateShadowTransform();

public:
	UILabel(UIElement *parent = nullptr) : 
		UIElement(parent), _colour(Colour::White), _shadowOffset(2.f, -2.f), _shadowColour(Colour::Black), _margin(4.f), _textSize(-1.f), _alignment(ETextAlignment::LEFT) {}
	virtual ~UILabel() {}

	const SharedPointer<const Font>& GetFont() const { return _font; }
	const Text& GetText() const { return _text; }
	const UIColour& GetColour() const { return _colour; }
	const Transform& GetRenderTransform() const { return _transform; }
	const Vector2& GetTextOffset() const { return _offset; }
	const Vector2& GetShadowOffset() const { return _shadowOffset; }
	const UIColour& GetShadowColour() const { return _shadowColour; }
	const ETextAlignment& GetAlignment() const { return _alignment; }
	float GetMargin() const { return _margin; }
	float GetTextSize() const { return _textSize; }

	UILabel& SetFont(const SharedPointer<const Font>& font) { _font = font; UpdateBounds(); return *this; }
	UILabel& SetText(const Text& text) { _text = text; UpdateBounds(); return *this; }
	UILabel& SetColour(const UIColour& colour) { _colour = colour; return *this; }
	UILabel& SetTextOffset(const Vector2& offset) { _offset = offset; UpdateBounds(); return *this; }
	UILabel& SetShadowOffset(const Vector2& offset) { _shadowOffset = offset; _UpdateShadowTransform(); return *this; }
	UILabel& SetShadowColour(const UIColour& colour) { _shadowColour = colour; return *this; }
	UILabel& SetAlignment(ETextAlignment alignment) { _alignment = alignment; UpdateBounds(); return *this; }
	UILabel& SetMargin(float margin) { _margin = margin; UpdateBounds(); return *this; }
	UILabel& SetTextSize(float textSize) { _textSize = textSize; UpdateBounds(); return *this; }

	virtual void UpdateBounds() override;
	virtual void Render(RenderQueue&) const override;

	virtual void Update(float deltaTime) override;
};
