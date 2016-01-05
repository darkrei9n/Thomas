#include "stdafx.h"



void DirectX12Render::DirectX12Render::Update()
{
}

void DirectX12Render::DirectX12Render::Initialize()
{
	InitializeDX();
}

void DirectX12Render::DirectX12Render::InitializeDX()
{
	//Initialize the debug layer of DX12
#ifdef _DEBUG
	ComPtr<ID3D12Debug> Debug;

	ThrowFail(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)), TEXT("Failed to get debug interface"));
	Debug->EnableDebugLayer();

#endif
	//Make a factory so we can get our device along with an adapter to store
	ComPtr<IDXGIFactory4> Factory;
	ThrowFail(CreateDXGIFactory1(IID_PPV_ARGS(&Factory)), TEXT("Failed to create factory"));

	ComPtr<IDXGIAdapter1> Adapter;
	ComPtr<IDXGIAdapter1> TempAdapter;
	//Get our device
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters1(i, &TempAdapter); ++i)
	{
		DXGI_ADAPTER_DESC1 Desc;
		TempAdapter->GetDesc1(&Desc);

		if (Desc.Flags == DXGI_ERROR_NOT_FOUND)
			continue;
		//Fake Create it to make sure it will actually work
		if (S_OK == D3D12CreateDevice(TempAdapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), NULL))
			break;
	}
	Adapter = TempAdapter.Detach();
	//Create the device for reals
	ThrowFail(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), &Device), TEXT("Failed to create DirectX 12 Device"));

	//Create Command Queue
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowFail(Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&CommandQueue)), TEXT("Failed to create Command Queue"));

	//Create the swap chain
	DXGI_SWAP_CHAIN_DESC SwapDesc = {};
	SwapDesc.BufferCount = FrameCount;
	SwapDesc.BufferDesc.Width = 0;
	SwapDesc.BufferDesc.Height = 0;
}

void DirectX12Render::DirectX12Render::InitializeAssets()
{
}

void DirectX12Render::DirectX12Render::SyncToGPU()
{
}
