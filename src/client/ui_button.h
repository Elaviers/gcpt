#pragma once
#include "ui_element.h"
#include "ui_label.h"
#include "ui_panel.h"
#include "ui_colour.h"
#include "shared/event.h"

class UIButton : public UIElement
{
protected:
	int _hold; //this is a bitfield. 0 == not held

	UIColour _colour;

	UIColourModifier _holdColourModify;
	UIColourModifier _focusColourModify;
	UIColourModifier _hoverColourModify;

	bool _colourDirty;

	UIPanel _panel;

	void _CalculateFinalColour();

public:
	Event<UIButton&> onPressed;

	UIButton(UIElement* parent = nullptr) : UIElement(parent), _hold(false), _panel(this), _colourDirty(true) 
	{ 
		UIElement::onFocusGained += Function(&UIButton::_CalculateFinalColour, *this);
		UIElement::onFocusLost += Function(&UIButton::_CalculateFinalColour, *this);
	}

	virtual ~UIButton() {}

	UIButton& SetColour(const UIColour& colour) { _colourDirty = true; _colour = colour; return *this; }
	UIColourModifier& ColourMod_Hold() { _colourDirty = true; return _holdColourModify; }
	UIColourModifier& ColourMod_Focus() { _colourDirty = true; return _focusColourModify; }
	UIColourModifier& ColourMod_Hover() { _colourDirty = true; return _hoverColourModify; }

	const UIColour& GetColour() const { return _colour; }
	float GetBorderSize(UIPanel::EBorder border) const { return _panel.GetBorderSize(border); }
	const SharedPointer<const Material>& GetMaterial() const { return _panel.GetMaterial(); }

	UIButton& SetBorderSize(UIPanel::EBorder borders, float borderSize) { _panel.SetBorderSize(borders, borderSize); return *this; }
	UIButton& SetMaterial(const SharedPointer<const Material>& material) { _panel.SetMaterial(material); return *this; }

	virtual void OnHoverStart() override;
	virtual void OnHoverStop() override;

	virtual void UpdateBounds() override
	{
		UIElement::UpdateBounds();
		_panel.UpdateBounds();	
	}

	virtual void Update(float deltaTime) override
	{
		if (_colourDirty)
		{
			_CalculateFinalColour();
			_colourDirty = false;
		}
	}

	virtual void Render(RenderQueue& q) const override
	{
		_panel.Render(q);
	}

	virtual bool OnKeyDown(bool blocked, EKeycode) override;
	virtual bool OnKeyUp(bool blocked, EKeycode) override;
};
