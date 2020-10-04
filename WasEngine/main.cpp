//**************************************************************************
//	Daniel Waseem, Copyright 2020
//
//************************************************************************
#include "stdafx.h"

HINSTANCE d_hInstance = NULL;
HWND d_hWND = NULL;
HANDLE hConsoleOut;
HANDLE hConsoleIn;

enum Color { BLUE = 1, GREEN = 2, RED = 4, YELLOW = 6, INTENSIFY = 8, DEFAULT = 7 };

#pragma region Forward Declarations
LRESULT CALLBACK WinProc(
	_In_  UINT uMsg,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
);

bool	GenerateConsole();
bool	GenerateWindow(WNDCLASSEX& wcex);
bool	GenerateHWND(LPCWSTR szClassName);
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

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
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
		inputDevices[0].hwndTarget = NULL;
		inputDevices[0].usUsage = 6;
		inputDevices[0].usUsagePage = 1;

		// RAW Mouse Input
		inputDevices[1].dwFlags = 0;
		inputDevices[1].hwndTarget = NULL;
		inputDevices[1].usUsage = 2;
		inputDevices[1].usUsagePage = 1;

		if (!RegisterRawInputDevices(inputDevices, 2, sizeof(RAWINPUTDEVICE)))
		{
			SetConsoleTextAttribute(hConsoleOut, RED | INTENSIFY);
			printf("Input Error! Error creating the input devices.\n");
		}
		else
		{
			SetConsoleTextAttribute(hConsoleOut, GREEN | INTENSIFY);
			printf("Success! Registered the input devices.\n");
		}
		SetConsoleTextAttribute(hConsoleOut, DEFAULT);
	}
	break;
	case WM_SIZE:
	{

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
	wcex.hbrBackground = NULL;
	wcex.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = NULL;
	wcex.hInstance = d_hInstance;
	wcex.hIconSm = NULL;
	wcex.lpfnWndProc = (WNDPROC)WinProc;
	wcex.lpszClassName = TEXT("DEngine");
	wcex.cbWndExtra = NULL;
	wcex.lpszMenuName = NULL;

	if (!RegisterClassEx(&wcex))
	{
		SetConsoleTextAttribute(hConsoleOut, RED | INTENSIFY);
		printf("Error 1 Window class was not created correctly!\n");
		SetConsoleTextAttribute(hConsoleOut, DEFAULT);
		return false;
	}
	else
	{
		SetConsoleTextAttribute(hConsoleOut, GREEN | INTENSIFY);
		printf("Success! Generated the window class\n");
		SetConsoleTextAttribute(hConsoleOut, DEFAULT);
		return true;
	}

}

bool	GenerateHWND(LPCWSTR szClassName)
{
	UINT winStyle, winStyleX;
	winStyle = WS_OVERLAPPEDWINDOW;
	winStyleX = NULL;

	RECT	window_size = { 0, 0, 800, 600 };
	if (!AdjustWindowRect(&window_size, winStyle, false))
	{
		printf("Error! Unable to adjust the window rect.\n");
		return false;
	}

	d_hWND = CreateWindowEx(winStyleX, szClassName, L"DEngine Main", winStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		window_size.right - window_size.left,
		window_size.bottom - window_size.top,
		NULL, NULL, d_hInstance, NULL);
	if (!d_hWND)
	{
		SetConsoleTextAttribute(hConsoleOut, RED | INTENSIFY);
		printf("HWND Error! HWND is null. Failed to create the HWND.\n");
		SetConsoleTextAttribute(hConsoleOut, DEFAULT);
		return false;
	}
	else
	{
		SetConsoleTextAttribute(hConsoleOut, GREEN | INTENSIFY);
		printf("Success Generated the HWND.\n");
		SetConsoleTextAttribute(hConsoleOut, DEFAULT);
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

	hConsoleOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hConsoleIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	SetStdHandle(STD_OUTPUT_HANDLE, hConsoleOut);
	SetStdHandle(STD_ERROR_HANDLE, hConsoleOut);
	SetStdHandle(STD_INPUT_HANDLE, hConsoleIn);
	std::wcout.clear();
	std::wclog.clear();
	std::wcerr.clear();
	std::wcin.clear();

	SetConsoleTitle(L"Debugger");

	return true;
}