#include "engine.h"

#include "imgui.h"
#include "console.h"
#include "convar.h"
#include "macro_utils.h"
#include "keycode.h"
#include "utils.h"

#ifdef BUILD_CLIENT
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_win32.h"
#include "client/gl.h"
#include "client/ui_text_button.h"
#include "client/ui_mainmenu.h"

LRESULT CALLBACK Engine_WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
#endif // BUILD_CLIENT

Engine::Engine() : _isRunning(false), _lastFrameDurationSeconds(1.f / 60.f)
{
	freetype = NULL;

#ifdef BUILD_CLIENT
	projection_world.SetType(EProjectionType::ORTHOGRAPHIC);
	projection_ui.SetType(EProjectionType::ORTHOGRAPHIC);
	projection_ui.SetNearFar(-100.f, 100.f);
	projection_ui.SetOrthographicScale(1.f);

	activeErrorDlg = nullptr;
#endif
}

bool Engine::_Startup()
{
	kickSets = new KickSetManager();
	kickSets->SetRootPath("data/kicksets/");
	kickSets->Initialise();

#ifdef BUILD_CLIENT
	IF_FIRST_EXEC_DO({
		WNDCLASSEX windowClass = {};
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.hInstance = ::GetModuleHandle(NULL);
		windowClass.lpszClassName = TEXT("EngineWindow");
		windowClass.lpfnWndProc = Engine_WindowProc;
		windowClass.hIcon = windowClass.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);
		::RegisterClassEx(&windowClass);
	});

	glContext.CreateDummyAndUse();
	GL::LoadDummyExtensions();

	window.Create( TEXT("EngineWindow"), TEXT("Gamma Cagadota Puyo Tetris"), WS_OVERLAPPEDWINDOW );
	glContext.Create(window, 4, 1);
	glContext.Use(window);
	GL::LoadExtensions(window);

	//OpenGL
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.1f, 0.f, 1.f);
	wglSwapIntervalEXT(0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glEnable(GL_MULTISAMPLE);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

	//freetpye
	FT_Error ftResult = FT_Init_FreeType(&freetype);
	if (ftResult)
		Debug::Error(CSTR("FT_Init_FreeType error 0x", String::FromInt(ftResult, 0, 16), ": ", FT_Error_String(ftResult)));


#ifdef BUILD_CLIENT
	//Imgui
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(window.GetHWND());
	ImGui_ImplOpenGL3_Init("#version 410");

	//Audio
	audioSession.Initialise();

	//Managers
	sounds = new SoundManager(audioSession.GetSampleRate());
	sounds->SetRootPath("data/audio/");
	sounds->Initialise();

	soundConfigurations = new SoundConfigManager();
	soundConfigurations->SetRootPath("data/audio/");
	soundConfigurations->Initialise();
#endif

	fonts = new FontManager();
	fonts->SetRootPath("data/fonts/");
	fonts->AddPath(Utilities::GetSystemFontDir());
	fonts->Initialise();

#ifdef BUILD_CLIENT
	meshes = new MeshManager();
	meshes->SetRootPath("data/meshes/");
	meshes->Initialise();

	textures = new TextureManager();
	textures->SetRootPath("data/materials/");
	textures->Initialise();

	materials = new MaterialManager();
	materials->SetRootPath("data/materials/");
	materials->Initialise(*textures);

	sheetFormats = new SheetFormatManager();
	sheetFormats->SetRootPath("data/sheetformats/");
	sheetFormats->Initialise();

	textManager = new TextManager();
	textManager->SetRootPath("data/text/");
	textManager->Initialise();

	inputManager = new InputManager();

	//Shaders
	shader_unlit.Load("data/shaders/unlit.vert", "data/shaders/unlit.frag");
	shader_id.Load("data/shaders/unlit.vert", "data/shaders/id.frag");

	defaultFont = fonts->Get("default");
	if (!defaultFont)
	{
		Debug::FatalError("Could not load default font!");
	}

	//Done
	window.Show();
#endif // BUILD_CLIENT

	return true;
}

void Engine::_Shutdown()
{
#ifdef BUILD_CLIENT
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	FT_Done_FreeType(freetype);

	delete sounds;
	delete soundConfigurations;
	delete fonts;
	delete meshes;
	delete textures;
	delete materials;
	delete sheetFormats;
	delete textManager;
	delete inputManager;
#endif

	delete kickSets;
}

void Engine::_Frame()
{
	_timeSeconds = (double)_timer.GetTicksSinceStart() / (double)_timer.GetTicksPerSecond();
	_timeMillis = _timer.GetTicksSinceStart() / (_timer.GetTicksPerSecond() / 1000);

#ifdef BUILD_CLIENT
	_HandleWindowEvents();

	if (canKillError && activeErrorDlg)
		KillError();

	//imgui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	_ImGui();

	audioSession.FillBuffer();

	bool cursorInWindow = true;
	{
		POINT cursorPoint;
		RECT clientRect;
		::GetCursorPos(&cursorPoint);
		::ScreenToClient(window.GetHWND(), &cursorPoint);
		::GetClientRect(window.GetHWND(), &clientRect);
		if (cursorPoint.x < 0 || cursorPoint.y < 0 || cursorPoint.x > clientRect.right || cursorPoint.y > clientRect.bottom)
			cursorInWindow = false;

		m_cursorPos.x = cursorPoint.x;
		m_cursorPos.y = clientRect.bottom - cursorPoint.y;
	}

	if (cursorInWindow)
	{
		uint32 hoverColour[4];
		fb_uiId.SampleIntegers(m_cursorPos.x, m_cursorPos.y, hoverColour);
		if (hoverColour[0] == 0)
			hoverColour[0] = -1;

		ui.SetCursorPos(m_cursorPos.x, m_cursorPos.y, hoverColour[0]);
	}
	else
		ui.SetCursorPos(m_cursorPos.x, m_cursorPos.y, 0);


	ui.Update(_lastFrameDurationSeconds);

	_Render();
#endif // BUILD_CLIENT
}

#ifdef BUILD_CLIENT
void Engine::_HandleWindowEvents()
{
	WindowEvent event;
	while (window.PollEvent(event))
	{
		switch (event.type)
		{
		case WindowEvent::CLOSED:
			_isRunning = false;
			break;

		case WindowEvent::RESIZE:
			projection_world.SetDimensions(Vector2T(event.data.resize.w, event.data.resize.h));
			projection_ui.SetDimensions(Vector2T(event.data.resize.w, event.data.resize.h));
			ui.SetBounds(UIBounds(0.f, 0.f, UICoord(0.f, event.data.resize.w), UICoord(0.f, event.data.resize.h)));
			fb_uiId.Create_RG32UI_Depth(event.data.resize.w, event.data.resize.h);
			break;

		case WindowEvent::LEFTMOUSEDOWN:
			if (!ui.OnKeyDown(false, EKeycode::MOUSE_LEFT))
				g_engine.inputManager->KeyDown(EKeycode::MOUSE_LEFT);
			break;
		case WindowEvent::LEFTMOUSEUP:
			ui.OnKeyUp(false, EKeycode::MOUSE_LEFT);
			g_engine.inputManager->KeyUp(EKeycode::MOUSE_LEFT);
			break;
		case WindowEvent::KEYDOWN:
			if (!event.data.keyDown.isRepeat)
				if (!ui.OnKeyDown(false, event.data.keyDown.key))
					g_engine.inputManager->KeyDown(event.data.keyDown.key);

			break;
		case WindowEvent::KEYUP:
			ui.OnKeyUp(false, event.data.keyUp.key);
			g_engine.inputManager->KeyUp(event.data.keyUp.key);
			break;

		case WindowEvent::FOCUS_LOST:
			g_engine.inputManager->Reset();
			break;
		}
	}
}

void Engine::_Render()
{
	glDepthFunc(GL_ALWAYS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ImGui::Render();

	rq_world.Clear();
	rq_world.CreateCameraEntry(projection_world, camera_world);
	//render world todo

	const Transform uiTransform = Transform(Vector3(projection_ui.GetDimensions().x / 2.f, projection_ui.GetDimensions().y / 2.f, 0.f));
	
	rq_ui.Clear();
	rq_ui.CreateCameraEntry(projection_ui, uiTransform);
	ui.Render(rq_ui);

	//Render data ready to go, let's do it
	shader_unlit.Use();
	rq_world.Render(ERenderChannels::BACKGROUND, meshes, textures, 0);
	rq_ui.Render(ERenderChannels::UI, meshes, textures, 0);
	rq_world.Render(ERenderChannels::ALL & ~ERenderChannels::BACKGROUND, meshes, textures, 0);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//UI idMap
	shader_id.Use();
	fb_uiId.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	rq_ui.Clear();
	rq_ui.CreateCameraEntry(projection_ui, uiTransform);
	ui.RenderID(rq_ui);
	rq_ui.Render(ERenderChannels::ALL, meshes, textures, 0);
	fb_uiId.Unbind();
		
	window.SwapBuffers();
}
#endif

int Engine::Run()
{
	if (_Startup())
	{
		_timer.Start();

		_isRunning = true;

#if BUILD_CLIENT
		MainMenu_Open();
#endif

		while (_isRunning)
		{
			_Frame();

			const uint64 tick = _timer.GetTicksSinceStart();
			_lastFrameDurationSeconds = (float)(tick - _lastFrameClockTick) / (float)_timer.GetTicksPerSecond();
			_lastFrameClockTick = tick;
		}

		_Shutdown();
	}

	return -1;
}

static ConVar sv_port("sv_port", "7777", "", ECVAR_REPLICATE);

#ifdef BUILD_CLIENT
static ConVar sv_local_clients("sv_local_clients", "0", "Amount of local clients to use when starting a listen server");
static void FullscreenConVarChanged();
ConVar cl_fullscreen("cl_fullscreen", "0", "Fullscreen mode", ECVAR_NO_FLAGS, FullscreenConVarChanged);

static void FullscreenConVarChanged()
{
	g_engine.window.SetBorderlessFullscreen(cl_fullscreen.GetBool());
}
#endif

void Engine::LaunchGame()
{
	StopGame();

	server.Start(sv_port.GetInt());

#ifdef BUILD_CLIENT
	client.ConnectLocal(&server);

	const int localPlayerCount = sv_local_clients.GetInt();
	localPlayers.SetSize(localPlayerCount);
	for (int i = 0; i < localPlayerCount; ++i)
	{
		localPlayers[i] = new Player(i);
		localPlayers[i]->Connect(client);
	}
#endif

	game.Initialise();
}


#ifdef BUILD_CLIENT
bool Engine::ConnectToGame(const Address& serverAddr)
{
	StopGame();

	const int localPlayerCount = sv_local_clients.GetInt();
	localPlayers.SetSize(localPlayerCount);
	for (int i = 0; i < localPlayerCount; ++i)
	{
		localPlayers[i] = new Player(i);
		localPlayers[i]->Connect(client);
	}

	return true;
}
#endif // BUILD_CLIENT

void Engine::StopGame()
{
#ifdef BUILD_CLIENT
	for (Player* player : localPlayers)
	{
		player->Disconnect();
		delete player;
	}

	localPlayers.Clear();
#endif

	server.Stop();
	game.Shutdown();

#ifdef BUILD_CLIENT
	ui.Clear();
#endif
}

void Engine::Error(const char* fmt, ...)
{
	StopGame();
	
	va_list vargs;

	va_start(vargs, fmt);
	Console_Msg("\n----------ENGINE ERROR----------\n");
	Console_VMsg(fmt, vargs);
	Console_Msg("\n--------------------------------\n");


	const int length = vsnprintf(nullptr, 0, fmt, vargs);
	String msgString('\0', length);
	vsnprintf(&msgString[0], length + 1, fmt, vargs);
	va_end(vargs);

#ifdef BUILD_CLIENT
	activeErrorDlg = new UI_ErrorDialog(Text::FromString(msgString));
	canKillError = false;
#else
	Debug::Error(msgString.begin());
#endif
}

ConCommand error("error", "Kicks player back to main menu with an error message", [](const ConArgs& args) { g_engine.Error(args.GetCommandString().begin()); });

#ifdef BUILD_CLIENT
void Engine::KillError()
{
	if (activeErrorDlg)
	{
		delete activeErrorDlg;
		activeErrorDlg = nullptr;

		ui.Clear();
		MainMenu_Open();
	}
}

void Engine::SetPlayerUids(const int* uids, int uidCount)
{
	if (uidCount != localPlayers.GetSize())
	{
		Error("Server did not provide a single uid for each player!");
	}

	for (int i = 0; i < localPlayers.GetSize(); ++i)
	{
		if (!localPlayers[i])
			Error("%s --- a player was not valid\n", __FUNCTION__);

		if (localPlayers[i]->m_uid != INVALID_USER)
			Error("%s --- player already had a valid uid! (%d)\n", __FUNCTION__, localPlayers[i]->m_uid);
		
		if (uids[i] == INVALID_USER)
			Error("%s --- Got invalid uid from server..? (%d)\n", __FUNCTION__, uids[i]);

		localPlayers[i]->m_uid = uids[i];
	}
}
#endif

#ifdef BUILD_CLIENT
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Engine_WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui::GetCurrentContext())
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
			return TRUE;

		ImGuiIO& io = ImGui::GetIO();

		if (io.WantCaptureMouse && WindowFunctions_Win32::IsMouseInput(msg))
			return 0;
		if (io.WantCaptureKeyboard && WindowFunctions_Win32::IsKeyInput(msg))
			return 0;
	}

	return WindowFunctions_Win32::WindowProc(hwnd, msg, wparam, lparam);
}
#endif // BUILD_CLIENT

#ifdef BUILD_CLIENT
void Engine::_ImGui(const char* windowName)
{
	Console_ImGui();

	static bool showDemo = false;
	static int additionalWindows = 0;

	if (ImGui::Begin(windowName))
	{
		ImGui::SetWindowPos(ImVec2(500, 300), ImGuiCond_FirstUseEver);
		ImGui::SetWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		ImGui::InputInt("Monitor count", &additionalWindows, 1, 1);

		if (StringUtils::Equal_CaseInsensitive(windowName, "engine"))
		{
			for (int i = 0; i < additionalWindows; ++i)
				_ImGui(CSTR("Engine (", i, ")"));
		}

		ImGui::Checkbox("ImGui demo window", &showDemo);

		if (showDemo)
			ImGui::ShowDemoWindow();

		if (ImGui::CollapsingHeader("Managers"))
			_ManagerImGui();

		if (ImGui::CollapsingHeader("AudioSession"))
			audioSession.ImGui();

		uint32 hoverID[4];
		hoverID[0] = (uint32)-1;

		fb_uiId.SampleIntegers(m_cursorPos.x, m_cursorPos.y, hoverID);
		
		ImGui::Text("Hover ID is %d", hoverID[0]);
	}

	ImGui::End();
}

#include "imgui_utils.h"

void Engine::_ManagerImGui()
{
	if (ImGui::TreeNode("KickSetManager"))
	{
		ImGuiUtil::AssetManagerImGui(*kickSets);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("SheetFormatManager"))
	{
		ImGuiUtil::AssetManagerImGui(*sheetFormats);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("SoundManager"))
	{
		ImGuiUtil::AssetManagerImGui(*sounds);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("SoundConfigManager"))
	{
		ImGuiUtil::AssetManagerImGui(*soundConfigurations);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("FontManager"))
	{
		ImGuiUtil::AssetManagerImGui(*fonts);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("MeshManager"))
	{
		ImGuiUtil::AssetManagerImGui(*meshes);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("MaterialManager"))
	{
		ImGuiUtil::AssetManagerImGui(*materials);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("TextureManager"))
	{
		ImGuiUtil::AssetManagerImGui(*textures);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("TextManager"))
	{
		ImGuiUtil::AssetManagerImGui(*textManager);
		ImGui::TreePop();
	}
}
#endif

static const char* s_keycodeNames[128] = {};
static Hashmap<Array<const char>, EKeycode> s_nameKeycodes;

template <size_t SZ>
void SetKeycodeName(EKeycode keycode, const char (&name)[SZ])
{
	s_keycodeNames[(int)keycode] = name;
	s_nameKeycodes[name] = keycode;
}

void InitKeycodeNames()
{
	IF_NOT_FIRST_EXEC_DO(return);

	SetKeycodeName(EKeycode::F1, "f1");
	SetKeycodeName(EKeycode::F2, "f2");
	SetKeycodeName(EKeycode::F3, "f3");
	SetKeycodeName(EKeycode::F4, "f4");
	SetKeycodeName(EKeycode::F5, "f5");
	SetKeycodeName(EKeycode::F6, "f6");
	SetKeycodeName(EKeycode::F7, "f7");
	SetKeycodeName(EKeycode::F8, "f8");
	SetKeycodeName(EKeycode::F9, "f9");
	SetKeycodeName(EKeycode::F10, "f10");
	SetKeycodeName(EKeycode::F11, "f11");
	SetKeycodeName(EKeycode::F12, "f12");
	SetKeycodeName(EKeycode::SPACE, "space");
	SetKeycodeName(EKeycode::LCTRL, "lctrl");
	SetKeycodeName(EKeycode::LSHIFT, "lshift");
	SetKeycodeName(EKeycode::LALT, "lalt");
	SetKeycodeName(EKeycode::RCTRL, "rctrl");
	SetKeycodeName(EKeycode::RSHIFT, "rshift");
	SetKeycodeName(EKeycode::RALT, "ralt");
	SetKeycodeName(EKeycode::ENTER, "enter");
	SetKeycodeName(EKeycode::SLASH, "/");
	SetKeycodeName(EKeycode::TILDE, "`");
	SetKeycodeName(EKeycode::SQBRACKETLEFT, "[");
	SetKeycodeName(EKeycode::SQBRACKETRIGHT, "]");
	SetKeycodeName(EKeycode::BACKSLASH, "|");
	SetKeycodeName(EKeycode::APOSTROPHE, "\'");
	SetKeycodeName(EKeycode::COMMA, ",");
	SetKeycodeName(EKeycode::PERIOD, ".");
	SetKeycodeName(EKeycode::INSERT, "ins");
	SetKeycodeName(EKeycode::HOME, "home");
	SetKeycodeName(EKeycode::PAGEUP, "pageup");
	SetKeycodeName(EKeycode::PAGEDOWN, "pagedn");
	SetKeycodeName(EKeycode::DEL, "del");
	SetKeycodeName(EKeycode::END, "end");
	SetKeycodeName(EKeycode::UP, "up");
	SetKeycodeName(EKeycode::DOWN, "down");
	SetKeycodeName(EKeycode::LEFT, "left");
	SetKeycodeName(EKeycode::RIGHT, "right");
	SetKeycodeName(EKeycode::MOUSE_LEFT, "mouseleft");
	SetKeycodeName(EKeycode::MOUSE_RIGHT, "mouseright");
	SetKeycodeName(EKeycode::MOUSE_MIDDLE, "mousemiddle");
	SetKeycodeName(EKeycode::MOUSE_4, "mouse4");
	SetKeycodeName(EKeycode::MOUSE_5, "mouse5");
	SetKeycodeName(EKeycode::MOUSE_SCROLLUP, "scrollup");
	SetKeycodeName(EKeycode::MOUSE_SCROLLDOWN, "scrolldn");
	SetKeycodeName(EKeycode::NP0, "np0");
	SetKeycodeName(EKeycode::NP1, "np1");
	SetKeycodeName(EKeycode::NP2, "np2");
	SetKeycodeName(EKeycode::NP3, "np3");
	SetKeycodeName(EKeycode::NP4, "np4");
	SetKeycodeName(EKeycode::NP5, "np5");
	SetKeycodeName(EKeycode::NP6, "np6");
	SetKeycodeName(EKeycode::NP7, "np7");
	SetKeycodeName(EKeycode::NP8, "np8");
	SetKeycodeName(EKeycode::NP9, "np9");
	SetKeycodeName(EKeycode::NP_ADD, "npadd");
	SetKeycodeName(EKeycode::NP_MUL, "npmul");
	SetKeycodeName(EKeycode::NP_SUB, "npsub");
	SetKeycodeName(EKeycode::NP_DIV, "npdiv");
	SetKeycodeName(EKeycode::NP_DEC, "npdec");
	SetKeycodeName(EKeycode::NP_ENTER, "npenter");
}

EKeycode KeycodeFromString(const char* string)
{
	if (string[0] && !string[1])
	{
		char c = string[0];
		if (c >= '0' && c <= '9')
			return (EKeycode)((int)EKeycode::NUM0 + c - '0');

		if (c >= 'A' && c <= 'Z')
			return (EKeycode)((int)EKeycode::A + c - 'A');

		if (c >= 'a' && c <= 'z')
			return (EKeycode)((int)EKeycode::A + c - 'a');
	}

	InitKeycodeNames();

	const Array<const char> arr = Array<const char>(string, StringUtils::Length(string) + 1);
	EKeycode* kc = s_nameKeycodes.TryGet(arr);
	if (kc)
		return *kc;

	return EKeycode::NONE;
}

#ifdef BUILD_CLIENT
ConCommand cc_bind("bind", "Bind a key to a command", [](const ConArgs& args) {

	if (args.GetArgCount() < 3)
	{
		Console_Msg("Not enough args\n");
		return;
	}

	EKeycode key = KeycodeFromString(args.GetArg(1).ToLower().begin());
	if (key == EKeycode::NONE)
	{
		Console_Msg("\"%s\" is not a valid key\n", args.GetArg(1).ToLower().begin());
		return;
	}

	const String& arg2 = args.GetArg(2);

	g_engine.inputManager->Unbind(key, EBindCategory::KEYDOWN);
	g_engine.inputManager->BindKeyDown(key, [arg2]() { Con_ExecuteCommand(arg2.begin()); });

});

ConCommand cc_input_reset("input_reset", "Reset input state", [](const ConArgs&) {
	g_engine.inputManager->Reset();

});
#endif
