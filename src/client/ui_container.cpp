#include "ui_container.h"
#include "shared/iterator_utils.h"
#include "shared/sorting.h"

void UIContainer::_OnChildGained(UIElement *child)
{
	for (size_t i = 0; i < _children.GetSize(); ++i)
		if (child->GetZ() > _children[i]->GetZ())
		{
			_children.Insert(i, child);
			return;
		}

	_children.Add(child);
}

void UIContainer::_OnChildLost(UIElement *child)
{
	auto indx = IteratorUtils::IndexOf(_children.begin(), _children.end(), child);
	if (indx != IteratorUtils::INVALID_INDEX)
		_children.Remove(indx);
}

void UIContainer::_SortChildren()
{
	Sorting::Quicksort<UIElement*, float>(_children.begin(), 0, _children.GetSize() - 1, [](UIElement*const& e) -> float { return -e->GetZ(); });
}

void UIContainer::Update(float deltaTime)
{
	for (uint32 i = 0; i < _children.GetSize(); ++i)
		if (_children[i]->IsMarkedForDelete())
		{
			UIElement *element = _children[i];
			element->SetParent(nullptr); //Also removes from children
			delete element;
		}
		else _children[i]->Update(deltaTime);
}

void UIContainer::Clear() 
{ 
	while (_children.GetSize() > 0) 
		_children[0]->SetParent(nullptr); 
}

UIElement* UIContainer::FindChildByID(uint32 id) const
{
	for (UIElement* child : _children)
	{
		if (child->GetID() == id)
			return child;
		
		if (UIContainer* containerChild = dynamic_cast<UIContainer*>(child))
			if (UIElement* result = containerChild->FindChildByID(id))
				return result;
	}

	return nullptr;
}

void UIContainer::FocusElement(UIElement* element)			
{ 
	for (UIElement* child : _children)
		child->FocusElement(element);
}

void UIContainer::UpdateBounds()
{
	UIElement::UpdateBounds();
	for (UIElement* child : _children)
		child->UpdateBounds();
}

void UIContainer::Render(RenderQueue& q) const
{ 
	for (UIElement* child : _children)
		child->Render(q);
}

void UIContainer::RenderID(RenderQueue& q) const
{
	for (UIElement* child : _children)
		child->RenderID(q);
}

bool UIContainer::OnKeyUp(bool blocked, EKeycode key)
{
	for (size_t i = _children.GetSize(); i > 0; --i)
		if (_children[i - 1]->OnKeyUp(blocked, key))
			blocked = true;

	return blocked;
}

bool UIContainer::OnKeyDown(bool blocked, EKeycode key)
{
	for (size_t i = _children.GetSize(); i > 0; --i)
		if (_children[i - 1]->OnKeyDown(blocked, key))
			blocked = true;

	if (!blocked)
	{
		for (const UIElement* child : _children)
		{
			if (child->HasFocus())
			{
				UIElement* nav = nullptr;
				switch (key)
				{
				case EKeycode::LEFT:
					nav = child->navLeft;
					break;
				case EKeycode::RIGHT:
					nav = child->navRight;
					break;
				case EKeycode::UP:
					nav = child->navUp;
					break;
				case EKeycode::DOWN:
					nav = child->navDn;
					break;
				}

				if (nav)
					nav->RequestFocus();

				break;
			}
		}
	}

	return blocked;
}

bool UIContainer::OnCharInput(bool blocked, char c)
{
	for (size_t i = _children.GetSize(); i > 0; --i)
		if (_children[i - 1]->OnCharInput(blocked, c))
			blocked = true;

	return blocked;
}

bool UIContainer::OnMouseMove(bool blocked, float x, float y, uint32 hoverId) 
{ 
	_hover = false;
	_cursor = ECursor::DEFAULT;

	for (size_t i = _children.GetSize(); i > 0; --i)
	{
		UIElement* child = _children[i - 1];

		if (child->OnMouseMove(blocked, x, y, hoverId))
			blocked = true;

		if (child->GetHover())
		{
			_hover = true;

			if ((_cursor == ECursor::ARROWS_HORIZONTAL && child->GetCursor() == ECursor::ARROWS_VERTICAL) ||
				(_cursor == ECursor::ARROWS_VERTICAL && child->GetCursor() == ECursor::ARROWS_HORIZONTAL))
				_cursor = ECursor::ARROWS_QUAD;
			else
				_cursor = child->GetCursor();
		}
	}

	_cursor = _cursor;
	return blocked;
}
