#pragma once
#include "stdafx.h"
#include "resource.h"
#include "DX12Render.h"

class CWindow
{
public:
	int APIENTRY TrueMain(_In_ HINSTANCE hInstance,
		_In_opt_ HINSTANCE hPrevInstance,
		_In_ LPWSTR    lpCmdLine,
		_In_ int       nCmdShow);
	ATOM MyRegisterClass(HINSTANCE hInstance);
	BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
	void ProcessCMDLine(_In_reads_(argc) WCHAR* argv[], int argc);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static HWND GethWnd();
	static int GetHeight() { return height; }
	static int GetWidth() { return width; }

private:
	static int height;
	static int width;

	static HWND hWnd;
	DirectX12Render Render;
	};
