#include "ui_toolbar.h"

void UIToolbar::_OnItemStateChanged(UICheckbox& pressedItem)
{
	static bool changing = false;
	if (!changing)
	{
		changing = true;
		UIToolbarItem* selectedItem = nullptr;

		for (size_t i = 0; i < _children.GetSize(); ++i)
		{
			UIToolbarItem* item = dynamic_cast<UIToolbarItem*>(_children[i]);
			if (item)
			{
				if (item == &pressedItem)
				{
					item->SetState(true);
					selectedItem = item;
				}
				else
					item->SetState(false);
			}
		}

		if (selectedItem)
			onItemSelected(*selectedItem);

		changing = false;
	}
}

void UIToolbar::AddButton(const Text& label, const SharedPointer<const Texture>& texture, uint16 userData)
{
	UIToolbarItem* item = new UIToolbarItem(this);
	item->SetUserData(userData).SetMaterial(_material).SetBorderSize(UIPanel::EBorder::ALL, _borderSize).SetTextureFalse(texture).SetTextureTrue(texture)
		.SetColourFalse(_colourFalse).SetColourTrue(_colourTrue);

	item->ColourMod_Hold() = UIColourModifier(EColourModifyMode::SET, _colourHold);
	item->ColourMod_Hover() = UIColourModifier(EColourModifyMode::SET, _colourHover);

	item->SetBounds(UIBounds(UICoord(0.f, _nextButtonX), 0.f, UICoord(0.f, _buttonW), 1.f));
	item->onStateChanged += Function(&UIToolbar::_OnItemStateChanged, *this);

	_nextButtonX += _buttonW + _buttonGap;
}

void UIToolbar::SelectByUserData(uint16 userData)
{
	for (size_t i = 0; i < _children.GetSize(); ++i)
	{
		UIToolbarItem* item = dynamic_cast<UIToolbarItem*>(_children[i]);
		if (item && item->GetUserData() == userData)
		{
			_OnItemStateChanged(*item);
			return;
		}
	}
}
