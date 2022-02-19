#include "ui_button.h"

void UIButton::_CalculateFinalColour()
{
	 UIColour finalColour = _colour;

	if (GetHover())
		_hoverColourModify.ModifyColour(finalColour);

	if (HasFocus())
		_focusColourModify.ModifyColour(finalColour);

	if (_hold != 0)
		_holdColourModify.ModifyColour(finalColour);

	_panel.SetColour(finalColour);
}

void UIButton::OnHoverStart()
{
	_colourDirty = true;
}

void UIButton::OnHoverStop()
{
	_colourDirty = true;
}

bool UIButton::OnKeyDown(bool blocked, EKeycode key)
{
	if (blocked) return false;

	int prevHold = _hold;
	if (_hover && key == EKeycode::MOUSE_LEFT)
		_hold |= 0x01;
	else if (HasFocus())
	{
		if (key == EKeycode::ENTER)
			_hold |= 0x02;
		else if (key == EKeycode::SPACE)
			_hold |= 0x04;
	}

	if (_hold != prevHold)
	{
		if (_hold)
		{
			if (!HasFocus())
				RequestFocus();

			_colourDirty = true;
		}

		return true;
	}

	if (_hold && (key == EKeycode::UP || key == EKeycode::DOWN))
		return true;

	return false;
}

bool UIButton::OnKeyUp(bool blocked, EKeycode key)
{
	if (_hold)
	{
		if (key == EKeycode::MOUSE_LEFT)
			_hold &= ~0x01;
		else if (key == EKeycode::ENTER)
			_hold &= ~0x02;
		else if (key == EKeycode::SPACE)
			_hold &= ~0x04;

		if (_hold == 0)
		{
			_colourDirty = true;
			onPressed(*this);
			return true;
		}
	}

	return false;
}
