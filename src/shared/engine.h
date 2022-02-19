#pragma once
#include "timer.h"


#include "kickset_manager.h"
#include "game.h"

#ifdef BUILD_CLIENT
#include "client/window_win.h"
#include "client/texture_manager.h"
#include "client/render_queue.h"
#include "client/gl_context.h"
#include "client/gl_framebuffer.h"
#include "client/gl_program.h"
#include "client/input_manager.h"
#include "client/material_manager.h"
#include "client/ui_container.h"
#include "client/ui_errordialog.h"
#include "client/sound_session.h"
#include "client/sound_manager.h"
#include "client/soundconfig_manager.h"
#include "projection.h"
#include "text_manager.h"
#include "mesh_manager.h"
#include "sheetformat_manager.h"

#endif

#include "font_manager.h"
#include <ft2build.h>
#include FT_FREETYPE_H

class Engine
{
	Timer _timer;
	bool _isRunning;

	uint64 _lastFrameClockTick;

	float _lastFrameDurationSeconds;

	double _timeSeconds; //since init
	uint64 _timeMillis; //since init

public:
	KickSetManager* kickSets;
	FontManager* fonts;
	FT_Library freetype;

#ifdef BUILD_CLIENT

	Window_Win32 window;

	//managers
	SoundManager* sounds;
	SoundConfigManager* soundConfigurations;
	MeshManager* meshes;
	MaterialManager* materials;
	TextureManager* textures;
	SheetFormatManager* sheetFormats;
	TextManager* textManager;
	InputManager* inputManager;

	UIContainer ui;

	GLContext glContext;

	Transform camera_world;

	Projection projection_world;
	Projection projection_ui;

	GLProgram shader_unlit;
	GLProgram shader_id;

	RenderQueue rq_world;
	RenderQueue rq_ui;

	GLFramebuffer fb_uiId;

	AudioSession audioSession;

	SharedPointer<const Font> defaultFont;

	UI_ErrorDialog* activeErrorDlg;
	bool canKillError;

	Client client;
#endif // BUILD_CLIENT

	Server server;
	Game game;

	Buffer<Player*> localPlayers;

private:
	bool _Startup();
	void _Shutdown();

	void _Frame();

#ifdef BUILD_CLIENT
	void _HandleWindowEvents();
	void _Render();

	void _ImGui(const char* windowName = "Engine");
	void _ManagerImGui();
#endif // BUILD_CLIENT

public:
	Engine();

	constexpr bool IsRunning() const { return _isRunning; }
	void Stop() { _isRunning = false; }

	int Run();

	constexpr float GetLastFrameTime() const { return _lastFrameDurationSeconds; }
	constexpr double GetTimeSeconds() const { return _timeSeconds; }
	constexpr uint64 GetTimeMillis() const { return _timeMillis; }

	Vector2T<uint16> m_cursorPos;


	//State
	void LaunchGame();
	
#ifdef BUILD_CLIENT
	bool ConnectToGame(const Address& serverAddr);
#endif

	void StopGame(); //disconnects / stops server

	void Error(const char* fmt, ...);

#ifdef BUILD_CLIENT
	void KillError();
	
	void SetPlayerUids(const int* uids, int uidCount);
#endif
};

extern Engine g_engine;
