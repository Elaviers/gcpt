#include "ui_menubutton.h"
#include "shared/maths.h"

void UI_MenuButton::GotoSelectedPos()
{
	_fromPos = Vector2(this->GetBounds().x.absolute, this->GetBounds().y.absolute);
	_targetPos = _rootPos + Vector2(100.f, 0.f);
	_posInterpAlpha = 0.f;
	_interpTimeSeconds = 0.33f;
}

void UI_MenuButton::GotoRootPos()
{
	_fromPos = Vector2(this->GetBounds().x.absolute, this->GetBounds().y.absolute);
	_targetPos = _rootPos;
	_posInterpAlpha = 0.f;
	_interpTimeSeconds = .33f;
}

UI_MenuButton::UI_MenuButton()
{
	_posInterpAlpha = 0.f;
	_interpTimeSeconds = 0.f;

	UIElement::onFocusGained += Function(&UI_MenuButton::GotoSelectedPos, *this);
	UIElement::onFocusLost += Function(&UI_MenuButton::GotoRootPos, *this);
}

void UI_MenuButton::OnHoverStart()
{
	UITextButton::OnHoverStart();

	
}

void UI_MenuButton::OnHoverStop()
{
	UITextButton::OnHoverStop();

}

void UI_MenuButton::Update(float deltaTime)
{
	UITextButton::Update(deltaTime);

	if (_posInterpAlpha < 1.f && _interpTimeSeconds)
	{
		_posInterpAlpha += deltaTime / _interpTimeSeconds;

		if (_posInterpAlpha > 1.f)
			_posInterpAlpha = 1.f;

		const float base = 1500000;
		//const float eval = Maths::Log((_posInterpAlpha * (base - 1)) + 1, base);
		const float eval = Maths::Pow(Maths::Sin(_posInterpAlpha * (Maths::PI_F / 2.f)), 1.f/2.f);
		const Vector2 evalPos = _fromPos + ((_targetPos - _fromPos) * eval);

		_updatingFromLerp = true;
		SetX(UICoord(GetBounds().x.relative, evalPos.x));
		SetY(UICoord(GetBounds().y.relative, evalPos.y));
		_updatingFromLerp = false;
	}
}

void UI_MenuButton::UpdateBounds()
{
	UITextButton::UpdateBounds();

	if (!_updatingFromLerp)
	{
		// -- HACKY HACK --
		// Whenever we start an animation, we set this to nonzero. We do NOT want to run this check if we have not animated (are at 0,0).
		if (_interpTimeSeconds != 0) {
			_interpTimeSeconds = 0.f; // Reset the time so we don't animate again

			// Position of the button before it moved
			const float eval = Maths::Pow(Maths::Sin(_posInterpAlpha * (Maths::PI_F / 2.f)), 1.f / 2.f);
			const Vector2 evalPos = _fromPos + ((_targetPos - _fromPos) * eval);

			// We use evalPos to check the calculated initial position of the button. 
			// If the current root position is not evalPos, we have moved and thus need to compensate for what our animation did and go back.
			if (_rootPos != evalPos) {
				const Vector2 delta = _rootPos - evalPos;

				_updatingFromLerp = true;
				SetX(UICoord(GetBounds().x.relative, GetBounds().x.absolute + delta.x));
				SetY(UICoord(GetBounds().y.relative, GetBounds().y.absolute + delta.y));
				_updatingFromLerp = false;
			}
		}
		// -- END HACK --

		_rootPos = Vector2(this->GetBounds().x.absolute, this->GetBounds().y.absolute);
	}
}
