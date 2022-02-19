#pragma once
#include "ui_text_button.h"

class UI_MenuButton : public UITextButton
{
	Vector2 _rootPos;
	Vector2 _fromPos;
	Vector2 _targetPos;
	float _posInterpAlpha;
	float _interpTimeSeconds;

	bool _updatingFromLerp;

	void GotoSelectedPos();
	void GotoRootPos();

public:
	UI_MenuButton();
	virtual ~UI_MenuButton() {}

	virtual void OnHoverStart() override;
	virtual void OnHoverStop() override;

	void Update(float deltaTime) override;

	void UpdateBounds() override;

};
