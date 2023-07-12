#pragma once

//--------------------------------------------------------------------------------------------------
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"

//--------------------------------------------------------------------------------------------------
class Game;

//--------------------------------------------------------------------------------------------------
class App
{
public:
	App();
	~App();

	void Startup();
	void Shutdown();
	void Run();
	void RunFrame();

	bool IsQuitting() const;
	void SetQuitting(bool isQuitting);

	bool HandleQuitRequested();
	void InputHandler();
	static bool Event_Quit(EventArgs& args);

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();


private:
	Camera		m_devConsoleCamera	= {};
	Game*		m_theGame			= nullptr;
	bool		m_isQuitting = false;
	bool		m_isSlowMo = false;
};