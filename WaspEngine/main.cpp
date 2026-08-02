//**************************************************************************
//	Daniel Waseem, Copyright 2020
//
//************************************************************************
#include "stdafx.h"
#include "Renderer.h"
#include "WaspLogger.h"

using namespace WaspLogger;

HINSTANCE d_hInstance = nullptr;
HWND d_hWND = nullptr;
HANDLE hConsoleOut;
HANDLE hConsoleIn;

Renderer g_renderer;

#pragma region Forward Declarations
LRESULT CALLBACK WinProc(
	_In_  HWND hwnd,
	_In_  UINT uMsg,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
);

bool	GenerateConsole();
bool	GenerateWindow(WNDCLASSEX& wcex);
bool	GenerateHWND(LPCWSTR szClassName);

void	Input();
void	Update(float deltaTime);
void	Render();
#pragma endregion

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ PSTR lpCmdLine, _In_ int nCmdShow)
{
	bool console = AllocConsole();
	GenerateConsole();

	//
	// Generate the Window
	//
	d_hInstance = hInstance;
	WNDCLASSEX wcex;

	if (!GenerateWindow(wcex))
	{
		return -1;
	}

	if (!GenerateHWND(wcex.lpszClassName))
	{
		return -1;
	}

	ShowWindow(d_hWND, SW_SHOW);
	UpdateWindow(d_hWND);

	if (!g_renderer.Init(d_hWND))
	{
		LogError("Renderer Error! Failed to initialize the OpenGL renderer.");
		return -1;
	}

	LARGE_INTEGER frequency;
	LARGE_INTEGER lastTime;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&lastTime);

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (msg.message != WM_QUIT)
	{
		// Drain all pending Windows messages without blocking so the loop can keep rendering.
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			break;
		}

		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);
		float deltaTime = static_cast<float>(currentTime.QuadPart - lastTime.QuadPart) / static_cast<float>(frequency.QuadPart);
		lastTime = currentTime;

		Input();
		Update(deltaTime);
		Render();
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WinProc(
	_In_ HWND hwnd,
	_In_  UINT uMsg,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
)
{
	// sort through and find what code to run for the message given
	switch (uMsg)
	{
	case WM_CREATE:
	{
		RAWINPUTDEVICE		inputDevices[2];

		// RAW Keyboard input
		inputDevices[0].dwFlags = 0;
		inputDevices[0].hwndTarget = nullptr;
		inputDevices[0].usUsage = 6;
		inputDevices[0].usUsagePage = 1;

		// RAW Mouse Input
		inputDevices[1].dwFlags = 0;
		inputDevices[1].hwndTarget = nullptr;
		inputDevices[1].usUsage = 2;
		inputDevices[1].usUsagePage = 1;

		if (!RegisterRawInputDevices(inputDevices, 2, sizeof(RAWINPUTDEVICE)))
		{
			LogError("Input Error! Error creating the input devices.");
		}
		else
		{
			LogInformation("Success! Registered the input devices.");
		}
	}
	break;
	case WM_SIZE:
	{
		g_renderer.Resize(LOWORD(lParam), HIWORD(lParam));
	}
	break;
	case WM_MOUSEMOVE:
	{

	}
	break;
	// this message is read when the window is closed
	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
	}
	break;
	case WM_DESTROY:
	{
		// close the application entirely
		g_renderer.Shutdown();
		FreeConsole();
		PostQuitMessage(0);
		return 0;
	}
	break;
	}

	// Handle any messages the switch statement didn't
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool	GenerateWindow(WNDCLASSEX& wcex)
{
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_OWNDC;
	wcex.hbrBackground = CreateSolidBrush(RGB(100, 100, 100));
	wcex.hCursor = (HCURSOR)LoadCursor(nullptr, IDC_ARROW);
	wcex.hIcon = nullptr;
	wcex.hInstance = d_hInstance;
	wcex.hIconSm = nullptr;
	wcex.lpfnWndProc = WinProc;
	wcex.lpszClassName = TEXT("WaspEngine");
	wcex.cbWndExtra = 0;
	wcex.lpszMenuName = nullptr;

	if (!RegisterClassEx(&wcex))
	{
		LogError("Error 1 Window class was not created correctly!");
		return false;
	}
	else
	{
		LogInformation("Success! Generated the window class");
		return true;
	}

}

bool	GenerateHWND(LPCWSTR szClassName)
{
	UINT winStyle, winStyleX;
	winStyle = WS_OVERLAPPEDWINDOW;
	winStyleX = 0;

	RECT	window_size = { 0, 0, 800, 600 };
	if (!AdjustWindowRect(&window_size, winStyle, false))
	{
		printf("Error! Unable to adjust the window rect.\n");
		return false;
	}

	d_hWND = CreateWindowEx(winStyleX, szClassName, L"Wasp Engine Main", winStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		window_size.right - window_size.left,
		window_size.bottom - window_size.top,
		nullptr, nullptr, d_hInstance, nullptr);
	if (!d_hWND)
	{
		LogError("HWND Error! HWND is null. Failed to create the HWND.");
		return false;
	}
	else
	{
		LogInformation("Success Generated the HWND.");
		return true;
	}
}

bool GenerateConsole()
{
	FILE* consoleFile;

	freopen_s(&consoleFile, "CONOUT$", "w", stdout);
	freopen_s(&consoleFile, "CONOUT$", "w", stderr);
	freopen_s(&consoleFile, "CONIN$", "r", stdin);
	std::cout.clear();
	std::clog.clear();
	std::cerr.clear();
	std::cin.clear();

	hConsoleOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	hConsoleIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	SetStdHandle(STD_OUTPUT_HANDLE, hConsoleOut);
	SetStdHandle(STD_ERROR_HANDLE, hConsoleOut);
	SetStdHandle(STD_INPUT_HANDLE, hConsoleIn);

	// Enable ANSI/VT100 escape sequence processing so the \033[...m color
	// codes used by LogInformation/LogWarning/LogError/LogDebug actually
	// change the text color instead of being printed as raw control chars.
	DWORD consoleMode = 0;
	if (GetConsoleMode(hConsoleOut, &consoleMode))
	{
		SetConsoleMode(hConsoleOut, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}
	std::wcout.clear();
	std::wclog.clear();
	std::wcerr.clear();
	std::wcin.clear();

	SetConsoleTitle(L"Debugger");

	return true;
}

void Input()
{
	// Poll arrow-key state and forward it to the renderer for movement.
	if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
	{
		LogInformation("Escape key pressed. Closing the application.");
		PostMessage(d_hWND, WM_CLOSE, 0, 0);
	}

	bool left = (GetAsyncKeyState(VK_LEFT) & 0x8000) != 0;
	bool right = (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0;
	bool up = (GetAsyncKeyState(VK_UP) & 0x8000) != 0;
	bool down = (GetAsyncKeyState(VK_DOWN) & 0x8000) != 0;

	if (left)
	{
		LogInformation("Left arrow key pressed.");
	}
	if (right)
	{
		LogInformation("Right arrow key pressed.");
	}
	if (up)
	{
		LogInformation("Up arrow key pressed.");
	}
	if (down)
	{
		LogInformation("Down arrow key pressed.");
	}

	g_renderer.SetMoveInput(left, right, up, down);
}

void Update(float deltaTime)
{
	g_renderer.Update(deltaTime);
}

void Render()
{
	g_renderer.Render();
}
