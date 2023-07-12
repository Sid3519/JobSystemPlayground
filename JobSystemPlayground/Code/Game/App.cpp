#include "Game/App.hpp"
#include "Game/Game.hpp"

//--------------------------------------------------------------------------------------------------
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"

//--------------------------------------------------------------------------------------------------
RandomNumberGenerator*	g_rng			= nullptr;
InputSystem*			g_theInput		= nullptr;
Renderer*				g_theRenderer	= nullptr;
Window*					g_theWindow		= nullptr;
App*					g_theApp		= nullptr;


//--------------------------------------------------------------------------------------------------
App::App()
{
	m_devConsoleCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(1920.f, 1080.f));
}


//--------------------------------------------------------------------------------------------------
App::~App()
{
}


//--------------------------------------------------------------------------------------------------
void App::Startup()
{
	JobSystemConfig jobSystemConfig;
	jobSystemConfig.m_numOfWorkerThreads = 5;
	g_theJobSystem = new JobSystem(jobSystemConfig);

	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem(inputSystemConfig);
	
	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "Protogame2D";
	windowConfig.m_clientAspect = 2.f;
	windowConfig.m_inputSystem = g_theInput;
	g_theWindow = new Window(windowConfig);
	
	RendererConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(renderConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_renderer = g_theRenderer;
	devConsoleConfig.m_camera = &m_devConsoleCamera;
	g_theDevConsole = new DevConsole(devConsoleConfig);

	// AudioSystemConfig audioConfig;
	// g_audio = new AudioSystem(audioConfig);

	g_theJobSystem->Startup();
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	// g_audio->Startup();
	g_rng = new RandomNumberGenerator();

	g_theEventSystem->SubscribeEventCallbackFunction("quit", Event_Quit);
	
	m_theGame = new Game();
	m_theGame->Startup();
}


//--------------------------------------------------------------------------------------------------
void App::Shutdown()
{	
	// g_audio->Shutdown();
	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->Shutdown();
	g_theDevConsole->Shutdown();
	g_theJobSystem->Shutdown();
	g_theEventSystem->Shutdown();

	// delete g_audio;
	// g_audio = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_theWindow;
	g_theWindow = nullptr;

	delete g_theInput;
	g_theInput = nullptr;

	delete g_theDevConsole;
	g_theDevConsole = nullptr;

	delete g_theEventSystem;
	g_theEventSystem = nullptr;

	delete g_theJobSystem;
	g_theJobSystem = nullptr;
}


//--------------------------------------------------------------------------------------------------
void App::Run()
{
	// Program main loop; keep running frames until it's time to quit
	while (!IsQuitting())
	{
		RunFrame();
	}
}


//--------------------------------------------------------------------------------------------------
void App::RunFrame()
{
	BeginFrame();
	
	InputHandler();
	Update();	
	Render();
	
	EndFrame();
}


//--------------------------------------------------------------------------------------------------
bool App::IsQuitting() const
{
	return m_isQuitting;
}


//--------------------------------------------------------------------------------------------------
void App::SetQuitting(bool isQuitting)
{
	m_isQuitting = isQuitting;
}


//--------------------------------------------------------------------------------------------------
bool App::HandleQuitRequested()
{
	SetQuitting(true);
	return false;
}


//--------------------------------------------------------------------------------------------------
void App::InputHandler()
{
	if (g_theInput->IsKeyDown(KEYCODE_ESC) && !g_theInput->IsKeyDown_WasDown(KEYCODE_ESC) ||
		g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::BACK))
	{
		if (m_theGame->GetCurrentState() == GAME_STATE_ATTRACT)
		{
			HandleQuitRequested();
			return;
		}

		if (m_theGame->GetCurrentState() == GAME_STATE_PLAYING)
		{
			m_theGame->SetDesiredState(GAME_STATE_ATTRACT);
		}
	}

	if (g_theInput->IsKeyDown_WasUp(KEYCODE_SPACE))
	{
		if (m_theGame->GetCurrentState() == GAME_STATE_ATTRACT)
		{
			m_theGame->SetDesiredState(GAME_STATE_PLAYING);
		}
	}
}


//--------------------------------------------------------------------------------------------------
bool App::Event_Quit(EventArgs& args)
{
	(void)args;
	g_theApp->HandleQuitRequested();
	return false;
}


//--------------------------------------------------------------------------------------------------
void App::BeginFrame()
{
	Clock::TickSystemClock();
	g_theJobSystem->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	// g_audio->BeginFrame();
}


//--------------------------------------------------------------------------------------------------
void App::Update()
{
	if (IsQuitting()) return;

	m_theGame->Update();
}


//--------------------------------------------------------------------------------------------------
void App::Render() const
{
	if (IsQuitting()) return;

	m_theGame->Render();
	AABB2 devConsoleBounds = AABB2(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));
	g_theDevConsole->Render(devConsoleBounds);
}


//--------------------------------------------------------------------------------------------------
void App::EndFrame()
{
	// g_audio->EndFrame();
	g_theInput->EndFrame();
	g_theDevConsole->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	g_theJobSystem->EndFrame();
}