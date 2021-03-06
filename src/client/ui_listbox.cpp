#include "ui_listbox.h"

void UIListBox::_UpdateLabels()
{
	size_t i = 1;
	for (UILabel& label : _labels)
	{
		label.SetBounds(UIBounds(0.f, UICoord(1.f, i * -_itemHeight), 1.f, UICoord(0.f, _itemHeight)));
		label.SetFont(_font);
		label.SetColour(_textColour);
		label.SetShadowColour(_textShadowColour);
		label.SetShadowOffset(_textShadowOffset);
		label.SetAlignment(_textAlignment);
		label.SetMargin(_textMargin);
		++i;
	}
}

void UIListBox::Add(const Text& item)
{
	size_t index = _labels.GetSize() + 1;

	UILabel& newLabel = _labels.EmplaceBack(this);
	newLabel.SetBounds(UIBounds(0.f, UICoord(1.f, index * -_itemHeight), 1.f, UICoord(0.f, _itemHeight)));
	newLabel.SetFont(_font);
	newLabel.SetColour(_textColour);
	newLabel.SetShadowColour(_textShadowColour);
	newLabel.SetShadowOffset(_textShadowOffset);
	newLabel.SetText(item);
}

bool UIListBox::OnKeyDown(bool blocked, EKeycode key)
{
	return key == EKeycode::MOUSE_LEFT && _hover;
}

bool UIListBox::OnKeyUp(bool blocked, EKeycode key)
{
	if (key == EKeycode::MOUSE_LEFT && _hover && _selectedLabel)
	{
		onSelectionSubmitted(*_selectedLabel);
	}

	return _hover;
}

bool UIListBox::OnMouseMove(bool blocked, float x, float y, uint32 hoverId)
{
	UIElement::OnMouseMove(blocked, x, y, hoverId);

	if (_hover)
	{
		for (UILabel& label : _labels)
		{
			if (label.OverlapsPoint(x, y, hoverId))
			{
				if (&label != _selectedLabel)
				{
					_selectedLabel = &label;
					_selectionBox.SetBounds(_selectedLabel->GetBounds());
					onSelectionChanged(*_selectedLabel);
				}

				break;
			}
		}	
	}
	else if (_selectedLabel)
	{
		_selectedLabel = nullptr;
		_selectionBox.SetBounds(UIBounds(0.f, 0.f, 0.f, 0.f));
	}

	return _hover;
}
