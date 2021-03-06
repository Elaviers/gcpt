#pragma once
#include "ui_text_button.h"
#include "ui_listbox.h"

//todo: make combobox a container, not a text button

class UIComboBox : public UITextButton
{
private:
	float _lastMouseX = 0.f, _lastMouseY = 0.f;
	uint32 _lastMouseHoverId = 0;

protected:
	bool _selecting;

	bool _readOnly;

	UIListBox _listBox;

	void _OnPressed(UIButton&);
	void _OnSelectionSubmitted(UILabel&);

	void _UpdateListBounds();

	void _StopSelecting();
public:
	Event<UIComboBox&> onSelectionChanged;

	UIComboBox(UIElement* parent = nullptr) : UITextButton(parent), _listBox(this), _selecting(false), _readOnly(false)
	{
		UITextButton::onPressed += Function(&UIComboBox::_OnPressed, *this);
		_listBox.onSelectionSubmitted += Function(&UIComboBox::_OnSelectionSubmitted, *this);
	}

	virtual ~UIComboBox() {}

	void Add(const Text& item) { _listBox.Add(item); _UpdateListBounds(); }

	bool GetReadOnly() const { return _readOnly; }
	float GetListZ() const { return _listBox.GetZ(); }
	float GetListBorderSize(UIPanel::EBorder border) const { return _listBox.GetBorderSize(border); }
	const SharedPointer<const Material>& GetListMaterial() const { return _listBox.GetMaterial(); }
	const UIColour& GetListColour() const { return _listBox.GetColour(); }
	float GetListItemHeight() const { return _listBox.GetItemHeight(); }
	const SharedPointer<const Font>& GetListFont() const { return _listBox.GetFont(); }
	const UIColour& GetListTextColour() const { return _listBox.GetTextColour(); }
	const UIColour& GetListTextShadowColour() const { return _listBox.GetTextShadowColour(); }
	const Vector2& GetListTextShadowOffset() const { return _listBox.GetTextShadowOffset(); }
	ETextAlignment GetListTextAlignment() const { return _listBox.GetTextAlignment(); }
	float GetListTextMargin() const { return _listBox.GetTextMargin(); }
	const UIColour& GetListSelectionColour() const { return _listBox.GetSelectionColour(); }

	UIComboBox& SetReadOnly(bool readOnly) { _readOnly = readOnly; return *this; }
	UIComboBox& SetListZ(float z) { _listBox.SetZ(z); return *this; }
	UIComboBox& SetListBorderSize(UIPanel::EBorder borders, float borderSize) { _listBox.SetBorderSize(borders, borderSize); return *this; }
	UIComboBox& SetListMaterial(const SharedPointer<const Material>& material) { _listBox.SetMaterial(material); return *this; }
	UIComboBox& SetListColour(const UIColour& colour) { _listBox.SetColour(colour); return *this; }
	UIComboBox& SetListItemHeight(float itemHeight) { _listBox.SetItemHeight(itemHeight); return *this; }
	UIComboBox& SetListFont(const SharedPointer<const Font>& font) { _listBox.SetFont(font); return *this; }
	UIComboBox& SetListTextColour(const UIColour& colour) { _listBox.SetTextColour(colour); return *this; }
	UIComboBox& SetListTextShadowColour(const UIColour& colour) { _listBox.SetTextShadowColour(colour); return *this; }
	UIComboBox& SetListTextShadowOffset(const Vector2& offset) { _listBox.SetTextShadowOffset(offset); return *this; }
	UIComboBox& SetListTextAlignment(ETextAlignment alignment) { _listBox.SetTextAlignment(alignment); return *this; }
	UIComboBox& SetListTextMargin(float margin) { _listBox.SetTextMargin(margin); return *this; }
	UIComboBox& SetListSelectionColour(const UIColour& colour) { _listBox.SetSelectionColour(colour); return *this; }

	virtual void UpdateBounds()
	{
		UITextButton::UpdateBounds();
		_listBox.UpdateBounds();
	}

	virtual void Render(RenderQueue& q) const override
	{
		UITextButton::Render(q);

		if (_selecting)
			_listBox.Render(q);
	}

	virtual bool OnKeyUp(bool blocked, EKeycode) override;
	virtual bool OnKeyDown(bool blocked, EKeycode) override;
	virtual bool OnMouseMove(bool blocked, float x, float y, uint32 hoverId) override;
};
