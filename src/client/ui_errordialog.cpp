#include "ui_errordialog.h"
#include "shared/engine.h"

UI_ErrorDialog::UI_ErrorDialog(const Text& msgString)
{
	constexpr const int panelw = 640;
	constexpr const int panelh = 480;

	_dlg.SetParent(&g_engine.ui);
	_dlg.SetBounds(UIBounds(UICoord(0.5, -panelw / 2), UICoord(0.5, -panelh / 2), UICoord(0, panelw), UICoord(0, panelh)));

	_panel.SetParent(&_dlg);
	_panel.SetBounds(UIBounds(0, 0, 1, 1));
	_panel.SetColour(UIColour(Colour(0.f, 0.f, 0.5f)));

	_title.SetParent(&_dlg);
	_title.SetFont(g_engine.defaultFont);
	_title.SetBounds(UIBounds(0, UICoord(1, -64), 1, UICoord(0, 64)));
	_title.SetText(Text::FromString("FATAL ENGINE ERROR"));
	_title.SetColour(UIColour(Colour::White));
	_title.SetShadowColour(UIColour(Colour::Red));

	_text.SetParent(&_dlg);
	_text.SetFont(g_engine.defaultFont);
	_text.SetBounds(UIBounds(0, UICoord(1, -64 - 64), 1, UICoord(0.f, 32.f)));
	_text.SetColour(UIColour(Colour::Grey));
	_text.SetShadowColour(UIColour(Colour::Black));
	_text.SetText(msgString);
	_text.SetTextSize(32.f);

	_btn.SetParent(&_dlg);
	_btn.SetFont(g_engine.defaultFont);
	_btn.SetBounds(UIBounds(0.125f, 0.1f, .25f, UICoord(0.f, 56.f)));
	_btn.SetTextColour(UIColour(Colour::White));
	_btn.SetText(Text::FromString("Return to Main Menu"));
	_btn.onPressed += [](const UIButton&)
	{
		g_engine.canKillError = true;
	};
}
