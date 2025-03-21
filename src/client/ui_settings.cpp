#ifdef BUILD_CLIENT
#include "ui_settings.h"
#include "ui_container.h"
#include "shared/engine.h"
#include "ui_checkbox.h"
#include "ui_menubutton.h"
#include "ui_mainmenu.h"
#include "ui_toolbar.h"

struct SettingsMenu
{
	UIContainer container;
	UIToolbar chk_fullscreen;
	UI_MenuButton btn_back;
};

static SettingsMenu g_settingsMenu;

void OnPressed_Back(UIButton&)
{
	MainMenu_Open();
}

void OnStateChanged_Fullscreen(UICheckbox& checkbox)
{
	if (checkbox.GetState())
	{
		// Enable fullscreen
		OutputDebugStringA("Fullscreen enabled\n");
	}
	else
	{
		// Disable fullscreen
		OutputDebugStringA("Fullscreen disabled\n");
	}
}

static void MenuButton_Init(UI_MenuButton& btn, const auto& font, int index)
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
	OutputDebugStringA("In settings init\n");
	AssertM(g_engine.IsRunning(), "SettingsMenu_Init: engine not running");

	auto font = g_engine.fonts->Get("menu");

	g_settingsMenu.chk_fullscreen.SetParent(&g_settingsMenu.container);
	g_settingsMenu.chk_fullscreen.AddButton(Text::FromString("Fullscreen"), g_engine.textures->Grey(), 0);
	g_settingsMenu.chk_fullscreen.SetBounds(UIBounds(UICoord(0, -200), UICoord(0.f, 1 * 70.f), UICoord(0.0f, 500.f + 1 * 64.f), UICoord(0.f, 64.f)));

	MenuButton_Init(g_settingsMenu.btn_back, font, 0);
	g_settingsMenu.btn_back.onPressed += OnPressed_Back;
}

void SettingsMenu_Open()
{
	OutputDebugStringA("In settings\n");
	IF_FIRST_EXEC_DO(SettingsMenu_Init());

	g_settingsMenu.container.SetParent(&g_engine.ui);
	g_engine.ui.FocusElement(&g_settingsMenu.btn_back);
}
#endif