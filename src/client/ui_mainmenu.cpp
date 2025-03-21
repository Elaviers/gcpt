#ifdef BUILD_CLIENT
#include "ui_container.h"
#include "shared/engine.h"
#include "ui_menubutton.h"
#include "ui_settings.h"

struct MainMenu
{
	UIContainer container;
	UI_MenuButton btn_play;
	UI_MenuButton btn_settings;
	UI_MenuButton btn_quit;
};

static MainMenu g_menu;

void MainMenu_Open();

void OnPressed_Play(UIButton&)
{

}

void OnPressed_Settings(UIButton&)
{
	OutputDebugStringA("Settings pressed\n");
	//g_engine.ui.Clear();
	g_menu.container.SetParent(nullptr);
	SettingsMenu_Open();
	//MainMenu_Open();
	
}

void OnPressed_Quit(UIButton&)
{
	g_engine.Stop();
}

static void MenuButton_Init(UI_MenuButton& btn, const auto& font, int index)
{
	btn.SetParent(&g_menu.container);
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

static void MainMenu_Init()
{
	AssertM(g_engine.IsRunning(), "MainMenu_Init: engine not running");

	auto font = g_engine.fonts->Get("menu");

	MenuButton_Init(g_menu.btn_quit, font, 0);
	MenuButton_Init(g_menu.btn_settings, font, 1);
	MenuButton_Init(g_menu.btn_play, font, 2);

	g_menu.btn_quit.navDn = &g_menu.btn_play;
	g_menu.btn_quit.SetText(Text::FromString("Quit!"));
	g_menu.btn_quit.onPressed += OnPressed_Quit;
	g_menu.btn_quit.navUp = &g_menu.btn_settings;

	g_menu.btn_settings.navDn = &g_menu.btn_quit;
	g_menu.btn_settings.SetText(Text::FromString("Settings!"));
	g_menu.btn_settings.onPressed += OnPressed_Settings;
	g_menu.btn_settings.navUp = &g_menu.btn_play;

	g_menu.btn_play.navDn = &g_menu.btn_settings;
	g_menu.btn_play.SetText(Text::FromString("Play!"));
	g_menu.btn_play.onPressed += OnPressed_Play;
	g_menu.btn_play.navUp = &g_menu.btn_quit;
}

void MainMenu_Open()
{
	IF_FIRST_EXEC_DO(MainMenu_Init());

	g_menu.container.SetParent(&g_engine.ui);

	g_engine.ui.FocusElement(&g_menu.btn_play);
}
#endif
