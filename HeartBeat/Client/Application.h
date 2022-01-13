#pragma once

class Client;

class Application
{
public:
	static int Run(HINSTANCE hInstance, int nCmdShow, int w = 800, int h = 600);

	static HWND GetHwnd() { return sHwnd; }
	static int GetScreenWidth() { return sScreenWidth; }
	static int GetScreenHeight() { return sScreenHeight; }
	static float GetAspectRatio() { return static_cast<float>(sScreenWidth) / static_cast<float>(sScreenHeight); }

protected:
	static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static void initWindow(HINSTANCE hInstance, int nCmdShow, int w, int h);

private:
	static HWND sHwnd;
	static int sScreenWidth;
	static int sScreenHeight;
};
