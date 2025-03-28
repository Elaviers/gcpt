#ifdef BUILD_CLIENT
#include "ui_container.h"
#include "shared/engine.h"
#include "ui_checkbox.h"
#include "ui_menubutton.h"
#include "ui_mainmenu.h"
#include "ui_toolbar.h"
#include "window.h"
#include <shared/convar.h>

struct SettingsMenu
{
	UIContainer container;
	UI_MenuButton chk_fullscreen;
	UI_MenuButton btn_back;
};

static SettingsMenu g_settingsMenu;

static void OnPressed_Back(UIButton&)
{
	g_settingsMenu.container.SetParent(nullptr);
	MainMenu_Open();
}

static void OnPressed_Fullscreen(UIButton&)
{
	extern ConVar cl_fullscreen;

	cl_fullscreen.SetBool(!cl_fullscreen.GetBool());
	g_settingsMenu.chk_fullscreen.SetText(Text::FromString((cl_fullscreen.GetBool() ? "Windowed" : "Fullscreen")));
}

static void SettingsMenuButton_Init(UI_MenuButton& btn, const auto& font, int index)
{
	btn.SetParent(&g_settingsMenu.container);
	btn.SetFont(font);
	btn.SetTextColour(UIColour(Colour::White));
	btn.SetTextShadowColour(UIColour(Colour::Grey));
	btn.SetBounds(UIBounds(UICoord(0, -200), UICoord(0.f, index * 70.f), UICoord(0.0f, 500.f + index * 64.f), UICoord(0.f, 64.f)));
	btn.SetTextBounds(UIBounds(UICoord(0, 200), UICoord(0), UICoord(1, 200), UICoord(1)));
	btn.SetMaterial(g_engine.materials->Get("ui/leftbutton"));
	btn.SetColour(UIColour(Colour::White, Colour::Black, Colour::Red));
	btn.SetBorderSize(UIPanel::EBorder::ALL, 4.f);
	btn.SetBorderSize(UIPanel::EBorder::RIGHT, 64.f);

	btn.ColourMod_Hold().SetSecondary(EColourModifyMode::SET, Colour(0.f, 0.f, 0.1f));
	btn.ColourMod_Focus().SetSecondary(EColourModifyMode::SET, Colour::Blue);
	btn.ColourMod_Hover().SetPrimary(EColourModifyMode::SET, Colour::Red);
}

static void SettingsMenu_Init()
{
	AssertM(g_engine.IsRunning(), "SettingsMenu_Init: engine not running");
	Debug::PrintLine("In settings init\n");

	auto font = g_engine.fonts->Get("menu");

	SettingsMenuButton_Init(g_settingsMenu.chk_fullscreen, font, 0);
	SettingsMenuButton_Init(g_settingsMenu.btn_back, font, 1);

	g_settingsMenu.chk_fullscreen.navDn = &g_settingsMenu.btn_back;
	g_settingsMenu.chk_fullscreen.SetText(Text::FromString("Fullscreen"));
	g_settingsMenu.chk_fullscreen.onPressed += OnPressed_Fullscreen;
	g_settingsMenu.chk_fullscreen.navUp = &g_settingsMenu.btn_back;
	
	// Make chk_fullscreen wider than whatever its current value is by 64 pixels
	UIBounds chk_fullscreen_bounds = g_settingsMenu.chk_fullscreen.GetBounds();
	chk_fullscreen_bounds.w.absolute += 64;
	g_settingsMenu.chk_fullscreen.SetBounds(chk_fullscreen_bounds);


	g_settingsMenu.btn_back.navDn = &g_settingsMenu.chk_fullscreen;
	g_settingsMenu.btn_back.SetText(Text::FromString("Back"));
	g_settingsMenu.btn_back.onPressed += OnPressed_Back;
	g_settingsMenu.btn_back.navUp = &g_settingsMenu.chk_fullscreen;
}

void SettingsMenu_Open()
{
	IF_FIRST_EXEC_DO(SettingsMenu_Init());
	Debug::PrintLine("In settings\n");

	g_settingsMenu.container.SetParent(&g_engine.ui);

	g_engine.ui.FocusElement(&g_settingsMenu.btn_back);
}
#endif