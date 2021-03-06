#include "ui_tab_book.h"
#include "shared/iterator_utils.h"

void UITabBook::_UpdateClientBounds()
{
	_clientBounds = _absoluteBounds;

	switch (_tabBarPosition)
	{
	case TabBarPosition::TOP:
		_clientBounds.h -= _tabBarSize;
		break;
	case TabBarPosition::RIGHT:
		_clientBounds.w -= _tabBarSize;
		break;
	case TabBarPosition::BOTTOM:
		_clientBounds.h -= _tabBarSize;
		_clientBounds.y += _tabBarSize;
		break;
	case TabBarPosition::LEFT:
		_clientBounds.w -= _tabBarSize;
		_clientBounds.x += _tabBarSize;
		break;
	}
}

void UITabBook::_OnChildGained(UIElement* child)
{
	if (_createTabsFromChildren && IteratorUtils::FirstWhere(_tabs.begin(), _tabs.end(), [child](const auto& it) { return it->page == child; }) == _tabs.end())
	{
		_UITabBookElement& tab = _tabs.EmplaceBack();
		tab.page = child;

		_createTabsFromChildren = false; //This is a little bit jank
		tab.label.SetParent(this);
		tab.panel.SetParent(this);
		_createTabsFromChildren = true;

		_UpdateTabs();
	}
}

void UITabBook::_OnChildLost(UIElement* child)
{
	_tabs.Remove(IteratorUtils::FirstWhere(_tabs.begin(), _tabs.end(), [child](const auto& it) { return it->page == child; }));

	_UpdateTabs();
}

void UITabBook::_UpdateTabs()
{
	float x = 0.f;

	for (_UITabBookElement& element : _tabs)
	{
		UIPanel& tab = element.panel;
		UILabel& lab = element.label;
		tab.SetBorderSize(UIPanel::EBorder::ALL, _tabBorderSize);
		lab.SetFont(_tabFont);

		float desiredWidth = lab.GetFont()->CalculateStringWidth(lab.GetText().ToString().begin(), lab.GetAbsoluteBounds().h);
		float w = Maths::Clamp(desiredWidth, _minimumTabSize, _maximumTabSize);

		UIBounds bounds(UICoord(0.f, x), UICoord(1.f, 0.f), UICoord(0.f, w), UICoord(0.f, _tabBarSize));
		tab.SetBounds(bounds);

		bounds.x.absolute += _tabBorderSize;
		lab.SetBounds(bounds);

		x += tab.GetBounds().w.absolute;

		if (&element == _activeTab)
		{
			tab.SetColour(_tabColourActive);
			tab.SetMaterial(_tabMaterialActive);
			lab.SetColour(_tabFontColourActive);
		}
		else if (&element == _hoverTab)
		{
			tab.SetColour(_tabColourHover);
			tab.SetMaterial(_tabMaterialHover);
			lab.SetColour(_tabFontColourHover);
		}
		else
		{
			tab.SetColour(_tabColour);
			tab.SetMaterial(_tabMaterial);
			lab.SetColour(_tabFontColour);
		}
	}
}

UIElement* UITabBook::GetTabWithName(const Text& tabName)
{
	for (const _UITabBookElement& element : _tabs)
		if (element.label.GetText() == tabName)
			return element.page;

	return nullptr;
}

void UITabBook::FocusElement(UIElement* element)
{
	if (_activeTab)
		_activeTab->page->FocusElement(element);
}

void UITabBook::UpdateBounds()
{
	UIElement::UpdateBounds();

	for (_UITabBookElement& element : _tabs)
	{
		element.page->UpdateBounds();
		element.panel.UpdateBounds();
		element.label.UpdateBounds();
	}
}


void UITabBook::Render(RenderQueue& renderQueue) const
{
	if (_activeTab)
		_activeTab->page->Render(renderQueue);

	for (const _UITabBookElement& element : _tabs)
	{
		element.panel.Render(renderQueue);
		element.label.Render(renderQueue);
	}
}

void UITabBook::Update(float deltaTime)
{
	if (_activeTab)
		_activeTab->page->Update(deltaTime);

	for (_UITabBookElement& element : _tabs)
	{
		element.panel.Update(deltaTime);
		element.label.Update(deltaTime);
	}
}

Text UITabBook::GetTabName(const UIElement& page) const
{
	if (page.GetParent() == this)
	{
		List<_UITabBookElement>::ConstIterator tab = IteratorUtils::FirstWhere(_tabs.begin(), _tabs.end(), [&page](const List<_UITabBookElement>::ConstIterator& element) { return element->page == &page; });
		
		if (tab == _tabs.end())
			return tab->label.GetText();
	}

	return Text();
}

bool UITabBook::SetActiveTab(const UIElement* page)
{
	if (page)
	{
		if (page->GetParent() == this)
		{
			List<_UITabBookElement>::Iterator tab = IteratorUtils::FirstWhere(_tabs.begin(), _tabs.end(), [&page](const List<_UITabBookElement>::Iterator& element) { return element->page == page; });

			if (tab != _tabs.end())
			{
				_activeTab = &*tab;
				_activeTab->page->UpdateBounds();
				_UpdateTabs();
				return true;
			}
		}
	}
	else
	{
		_activeTab = nullptr;
		_UpdateTabs();
		return true;
	}

	return false;
}

bool UITabBook::SetTabName(const UIElement& page, const Text& name)
{
	if (page.GetParent() == this)
	{
		List<_UITabBookElement>::Iterator tab =
			IteratorUtils::FirstWhere(
				_tabs.begin(), _tabs.end(),
				[&page](const List<_UITabBookElement>::Iterator& element) { return element->page == &page; });
		
		if (tab != _tabs.end())
		{
			tab->label.SetText(name);
			return true;
		}
	}

	return false;
}

bool UITabBook::OnKeyUp(bool blocked, EKeycode key)
{
	if (key == EKeycode::MOUSE_LEFT && _hoverTab)
		blocked = true;

	if (_activeTab)
		return _activeTab->page->OnKeyUp(blocked, key);

	return blocked;
}

bool UITabBook::OnKeyDown(bool blocked, EKeycode key)
{
	if (key == EKeycode::MOUSE_LEFT && _hoverTab)
	{
		blocked = true;
		_activeTab = _hoverTab;
		_UpdateTabs();
	}

	if (_activeTab)
		return _activeTab->page->OnKeyDown(blocked, key);

	return blocked;
}

bool UITabBook::OnCharInput(bool blocked, char c)
{
	if (_activeTab)
		return _activeTab->page->OnCharInput(blocked, c);

	return blocked;
}

bool UITabBook::OnMouseMove(bool blocked, float x, float y, uint32 hoverId)
{
	UIElement::OnMouseMove(blocked, x, y, hoverId);

	_hoverTab = nullptr;
	for (_UITabBookElement& e : _tabs)
	{
		if (e.panel.OverlapsPoint(x, y, hoverId))
		{
			blocked = true;
			_hoverTab = &e;
			_UpdateTabs();
			_cursor = ECursor::DEFAULT;
		}
	}

	if (_activeTab)
	{
		if (_activeTab->page->OnMouseMove(blocked, x, y, hoverId))
		{
			_cursor = _activeTab->page->GetCursor();
			blocked = true;
		}
	}

	return blocked;
}
