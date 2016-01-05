#pragma once
#include <Windows.h>
//Currently only exceptions are thrown because its nice to know Windows or DirectX fucked itself and threw something

namespace Exceptions
{
	inline void ThrowFail(HRESULT hr, LPCWSTR error)
	{
		if (FAILED(hr))
		{
			MessageBox(NULL, error, 0, 0);
		}
	}
}
