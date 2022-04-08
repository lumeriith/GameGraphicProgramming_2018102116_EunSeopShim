#include "Game/Game.h"

namespace library {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::Game

	  Summary:  Constructor

	  Args:     PCWSTR pszGameName
				  Name of the game

	  Modifies: [m_pszGameName, m_mainWindow, m_renderer].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	Game::Game(_In_ PCWSTR pszGameName) :
		m_pszGameName(pszGameName),
		m_mainWindow(),
		m_renderer()
	{}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::Initialize

	  Summary:  Initializes the components of the game

	  Args:     HINSTANCE hInstance
				  Handle to the instance
				INT nCmdShow
				  Is a flag that says whether the main application window
				  will be minimized, maximized, or shown normally

	  Modifies: [m_mainWindow, m_renderer].

	  Returns:  HRESULT
				Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT library::Game::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow) {
		HRESULT hr;

		m_mainWindow = std::make_unique<MainWindow>();
		hr = m_mainWindow->Initialize(hInstance, nCmdShow, L"Game Graphics Programming Lab 02: Object Oriented Design");
		if (FAILED(hr)) return hr;

		m_renderer = std::make_unique<Renderer>();
		hr = m_renderer->Initialize(m_mainWindow->GetWindow());
		if (FAILED(hr)) return hr;

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::Run

	  Summary:  Runs the game loop

	  Returns:  INT
				  Status code to return to the operating system
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	INT Game::Run() {
		MSG msg = {};
		PeekMessage(&msg, nullptr, 0U, 0U, PM_NOREMOVE);

		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);

		LARGE_INTEGER startingTicks, endingTicks;
		LARGE_INTEGER elapsedMicroseconds = {};

		QueryPerformanceCounter(&startingTicks);
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE) != 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				QueryPerformanceCounter(&endingTicks);
				elapsedMicroseconds.QuadPart = endingTicks.QuadPart - startingTicks.QuadPart;
				elapsedMicroseconds.QuadPart *= 1000000;
				elapsedMicroseconds.QuadPart /= frequency.QuadPart;
				// Update game
				m_renderer->Update(elapsedMicroseconds.QuadPart / 1000000.0f);
				QueryPerformanceCounter(&startingTicks);
				m_renderer->Render();
			}
		}

		return static_cast<INT>(msg.wParam);
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::GetGameName

	  Summary:  Returns the name of the game

	  Returns:  PCWSTR
				  Name of the game
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	PCWSTR Game::GetGameName() const {
		return m_pszGameName;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::GetWindow

	  Summary:  Returns the main window

	  Returns:  std::unique_ptr<MainWindow>&
				  The main window
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	std::unique_ptr<MainWindow>& Game::GetWindow()
	{
		return m_mainWindow;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::GetRenderer

	  Summary:  Returns the renderer

	  Returns:  std::unique_ptr<Renderer>&
				  The renderer
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	std::unique_ptr<Renderer>& Game::GetRenderer()
	{
		return m_renderer;
	}
}