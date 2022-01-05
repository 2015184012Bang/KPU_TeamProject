#include "ClientPCH.h"
#include "Application.h"

#include "Client.h"

HWND Application::sHwnd;
int Application::sScreenWidth;
int Application::sScreenHeight;

int Application::Run(HINSTANCE hInstance, int nCmdShow, int w /*= 800*/, int h /*= 600*/)
{
	unique_ptr<Client> client = std::make_unique<Client>();

	initWindow(hInstance, nCmdShow, w, h);

	bool retval = client->Init();

	if (retval == false)
	{
		return -1;
	}

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			client->Run();

			if (client->ShouldClose())
			{
				break;
			}
		}
	}

	client->Shutdown();

	return static_cast<char>(msg.wParam);
}

LRESULT CALLBACK Application::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

void Application::initWindow(HINSTANCE hInstance, int nCmdShow, int w, int h)
{
	sScreenWidth = w;
	sScreenHeight = h;

	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = windowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"HeartBeat";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, w, h };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	sHwnd = CreateWindow(
		windowClass.lpszClassName,
		L"HeartBeat",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

#ifdef _DEBUG
	AllocConsole();
#endif
	ShowWindow(sHwnd, nCmdShow);
}