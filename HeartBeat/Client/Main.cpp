#include "ClientPCH.h"

#include "Application.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	return Application::Run(hInstance, nCmdShow);
}