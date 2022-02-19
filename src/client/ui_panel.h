#pragma once
#include "ui_element.h"
#include "ui_colour.h"
#include "shared/shared_ptr.h"
#include "shared/transform.h"
#include "material.h"

class UIPanel : public UIElement
{
public:
	enum class EBorder
	{
		TOP = 0x01,
		RIGHT = 0x02,
		BOTTOM = 0x04,
		LEFT = 0x08,

		ALL = 0xFF
	};

private:
	SharedPointer<const Material> _material;
	UIColour _colour;

	float _borderSizes[4];

	Transform _transforms[9];

	Vector2 _uvScaling;
public:
	UIPanel(UIElement* parent = nullptr) : UIElement(parent), _colour(Colour::White) { _borderSizes[0] = _borderSizes[1] = _borderSizes[2] = _borderSizes[3] = 8.f; }
	virtual ~UIPanel() {}

	virtual void UpdateBounds() override;
	virtual void Render(RenderQueue&) const override;

	float GetBorderSize(EBorder border) const;
	const SharedPointer<const Material>& GetMaterial() const { return _material; }
	const UIColour& GetColour() const { return _colour; }

	UIPanel& SetBorderSize(EBorder borders, float size);
	UIPanel& SetColour(const UIColour& colour) { _colour = colour; return *this; }
	UIPanel& SetMaterial(const SharedPointer<const Material>& material) { _material = material; return *this; }
};

DEFINE_BITMASK_FUNCS(UIPanel::EBorder, byte);
