#pragma once

class InputSystem;

struct WindowConfig
{
	InputSystem* m_inputSystem = nullptr;
	std::string m_windowTitle = "Untitled App";
	float m_clientAspect = 1.f;
	bool m_isFullScreen = false;
};

class Window
{
public:
	Window(WindowConfig const& config);
	~Window();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	WindowConfig const& GetConfig() const { return m_config; }
	static Window* GetWindowContext() { return s_mainWindow; }
	bool IsWindowInFocus();

	void* GetHwnd() const; // Handle to a window
	IntVec2 GetClientDimensions() const;

	Vec2 GetNormalizedCursorPos() const; // UV's for the client screen

protected:
	void CreateOSWindow();
	void RunMessagePump();

protected:
	void* m_windowHandle = {};
	float m_clientWidth =	-1.f;
	float m_clientHeight =	-1.f;
	WindowConfig m_config;
	static Window* s_mainWindow;
};