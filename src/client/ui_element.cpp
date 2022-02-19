#include "ui_element.h"
#include "ui_container.h"
#include "render_queue.h"

uint32 UIElement::_sNextId = 1;

UIElement::UIElement(UIElement* parent) : _parent(parent), _id(_sNextId++), _markedForDelete(false), _hasFocus(false), _focusOnClick(true), _hover(false), _z(0.f), _cursor(ECursor::DEFAULT),
	navUp(nullptr), navDn(nullptr), navLeft(nullptr), navRight(nullptr)
{
	if (_parent)
	{
		UpdateBounds();
		_parent->_OnChildGained(this);
	}
}

UIElement::~UIElement()
{
	if (_parent)
		_parent->_OnChildLost(this);
}

UIElement& UIElement::SetParent(UIElement* parent)
{
	if (_parent)
		_parent->_OnChildLost(this);

	_parent = parent;
	UpdateBounds();

	if (_parent)
		_parent->_OnChildGained(this);

	return *this;
}

UIElement& UIElement::SetZ(float z)
{
	_z = z; 
	UpdateBounds(); 
	
	UIContainer* p = dynamic_cast<UIContainer*>(_parent);
	if (p) p->_SortChildren();

	return *this;
}

void UIElement::RequestFocus()
{
	if (_parent)
	{
		UIElement* root = _parent;
		while (root->_parent)
			root = root->_parent;

		root->FocusElement(this);
	}
}

void UIElement::FocusElement(UIElement* element)
{
	bool shouldHaveFocus = (element == this);
	
	if (_hasFocus != shouldHaveFocus)
	{
		_hasFocus = shouldHaveFocus;

		if (_hasFocus)
			onFocusGained();
		else
			onFocusLost();
	}
}

void UIElement::UpdateBounds()
{
	if (_parent)
	{
		_absoluteBounds.x = _parent->_clientBounds.x + (_parent->_clientBounds.w * _bounds.x.relative) + _bounds.x.absolute;
		_absoluteBounds.y = _parent->_clientBounds.y + (_parent->_clientBounds.h * _bounds.y.relative) + _bounds.y.absolute;
		_absoluteBounds.w = _parent->_clientBounds.w * _bounds.w.relative + _bounds.w.absolute;
		_absoluteBounds.h = _parent->_clientBounds.h * _bounds.h.relative + _bounds.h.absolute;
	}
	else
	{
		_absoluteBounds.x = _bounds.x.relative + _bounds.x.absolute;
		_absoluteBounds.y = _bounds.y.relative + _bounds.y.absolute;
		_absoluteBounds.w = _bounds.w.relative + _bounds.w.absolute;
		_absoluteBounds.h = _bounds.h.relative + _bounds.h.absolute;
	}

	_UpdateClientBounds();
}

void UIElement::_UpdateClientBounds()
{
	_clientBounds = _absoluteBounds;
}

void UIElement::RenderID(RenderQueue& q) const
{
	uint32 colour[4] = { _id, 0, 0, 0xFFFFFFFF };
	q.CreateEntry(ERenderChannels::ALL).AddSetUVec4(RCMDSetUVec4::Type::COLOUR, colour);
	Render(q);
}

bool UIElement::OnKeyDown(bool blocked, EKeycode key)
{
	if (key == EKeycode::MOUSE_LEFT && _focusOnClick && _hover)
	{
		RequestFocus();
		return true;
	}

	return false;
}

bool UIElement::OnKeyUp(bool blocked, EKeycode key)
{
	return key == EKeycode::MOUSE_LEFT && _focusOnClick && _hover;
}

bool UIElement::OnMouseMove(bool blocked, float x, float y, uint32 hoverId)
{
	if (blocked)
	{
		if (_hover)
		{
			_hover = false;
			OnHoverStop();
		}

		return false;
	}

	bool hover = OverlapsPoint(x, y, hoverId);
	if (hover != _hover)
	{
		_hover = hover;
		_hover ? OnHoverStart() : OnHoverStop();
	}

	return _hover;
}
