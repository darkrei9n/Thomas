#pragma once

#include "resource.h"

namespace WindowsApplication
{
	class CWindow
	{
	public:
		int APIENTRY TrueMain(_In_ HINSTANCE hInstance,
			_In_opt_ HINSTANCE hPrevInstance,
			_In_ LPWSTR    lpCmdLine,
			_In_ int       nCmdShow);
		ATOM MyRegisterClass(HINSTANCE hInstance);
		BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		HWND hWNd;
		DirectX12Render::DirectX12Render Render;
	};
}