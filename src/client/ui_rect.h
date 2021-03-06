#pragma once
#include "ui_element.h"
#include "ui_colour.h"
#include "shared/shared_ptr.h"
#include "shared/transform.h"
#include "texture.h"

class UIRect : public UIElement
{
protected:
	SharedPointer<const Texture> _texture;

	UIColour _colour;

	Transform _transform;

public:
	UIRect(UIElement *parent = nullptr) : UIElement(parent), _colour(Colour::White) {}
	virtual ~UIRect() {}

	const SharedPointer<const Texture>& GetTexture() const { return _texture; }
	const UIColour& GetColour() const { return _colour; }
	const Transform& GetRenderTransform() const { return _transform; }

	UIRect& SetTexture(const SharedPointer<const Texture>& texture) { _texture = texture; return *this; }
	UIRect& SetColour(const UIColour& colour) { _colour = colour; return *this; }

	virtual void UpdateBounds() override;
	virtual void Render(RenderQueue&) const override;
};
