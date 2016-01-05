#include "stdafx.h"
#include "DX12Render.h"


void DirectX12Render::Update()
{
}

void DirectX12Render::Initialize(UINT width, UINT height)
{
	Width = width;
	Height = height;
	InitializeDX();
	InitializeAssets();
}

void DirectX12Render::InitializeDX()
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
	SwapDesc.BufferDesc.Width = CWindow::GetWidth();
	SwapDesc.BufferDesc.Height = CWindow::GetHeight();
	SwapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapDesc.OutputWindow = CWindow::GethWnd();
	SwapDesc.SampleDesc.Count = 1;
	SwapDesc.Windowed = true;

	//Create a temporary swap chain because CreateSwapChain doesnt support SwapChain3
	ComPtr<IDXGISwapChain> FakeSwap;
	ThrowFail(Factory->CreateSwapChain(
		CommandQueue.Get(),
		&SwapDesc,
		&FakeSwap), TEXT("Fake Swap Chain Failed to create"));
	//.As() makes its TARGET itself not the otherway around. Causes evil shit if you do it the other way
	ThrowFail(FakeSwap.As(&SwapChain), TEXT("Real Swap Chain failed to switch"));
	
	FenceIndex = SwapChain->GetCurrentBackBufferIndex();

	D3D12_DESCRIPTOR_HEAP_DESC RTVDesc = {};
	RTVDesc.NumDescriptors = FrameCount;
	RTVDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowFail(Device->CreateDescriptorHeap(&RTVDesc, IID_PPV_ARGS(&RTVHeap)), TEXT("Descriptor Heap failed to create"));

	HeapSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle( RTVHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < FrameCount; i++)
	{
		ThrowFail(SwapChain->GetBuffer(i, IID_PPV_ARGS(&RenderTargets[i])), TEXT("Failed to Get Buffer"));
		Device->CreateRenderTargetView(RenderTargets[i].Get(), nullptr, RTVHandle);
		RTVHandle.Offset(1, HeapSize);
	}

	ThrowFail(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)), TEXT("Command Allocator Failed to Initialize"));
}

void DirectX12Render::DirectX12Render::InitializeAssets()
{
	//Create the root sig
	CD3DX12_ROOT_SIGNATURE_DESC RootDesc;

	RootDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> Signature;
	ComPtr<ID3DBlob> Error;

	ThrowFail(D3D12SerializeRootSignature(&RootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &Signature, &Error), TEXT("Root Signature Failed to Create"));
	ThrowFail(Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)), TEXT("Root Signature Failed to Create"));

	ComPtr<ID3DBlob> VertexShader;
	ComPtr<ID3DBlob> PixelShader;
	//Shader Debugging code
#if defined(_DEBUG)
	UINT CompileFlags = 1 << 0 | 1 << 2;
#else
	UINT CompileFlags = 0;
#endif

	ThrowFail(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", CompileFlags, 0, &VertexShader, nullptr), TEXT("Failed to Compile Vertex Shader"));
	ThrowFail(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", CompileFlags, 0, &PixelShader, nullptr), TEXT("Failed to Compile Pixel Shader"));

	D3D12_INPUT_ELEMENT_DESC InputElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	//Create the Pipeline State Object

	D3D12_GRAPHICS_PIPELINE_STATE_DESC PSO = {};
	PSO.InputLayout = { InputElementDesc, _countof(InputElementDesc) };
	PSO.pRootSignature = RootSignature.Get();
	PSO.VS = { reinterpret_cast<UINT8*>(VertexShader->GetBufferPointer()), VertexShader->GetBufferSize() };
	PSO.PS = { reinterpret_cast<UINT8*>(PixelShader->GetBufferPointer()), PixelShader->GetBufferSize() };
	PSO.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	PSO.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	PSO.DepthStencilState.DepthEnable = false;
	PSO.DepthStencilState.StencilEnable = false;
	PSO.SampleMask = UINT_MAX;
	PSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PSO.NumRenderTargets = 1;
	PSO.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	PSO.SampleDesc.Count = 1;
	ThrowFail(Device->CreateGraphicsPipelineState(&PSO, IID_PPV_ARGS(&Pipeline)), TEXT("Failed to create Pipeline"));

	//Create Command list
	ThrowFail(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), Pipeline.Get(), IID_PPV_ARGS(&CommandList)), TEXT("Command List Creation Failed"));

	ThrowFail(CommandList->Close(), TEXT("Command List Failed to Close"));

	//Create a fence to sync CPU to GPU
	ThrowFail(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)), TEXT("Failed to create fence"));

	FenceHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (FenceHandle == nullptr)
	{
		ThrowFail(HRESULT_FROM_WIN32(GetLastError()), TEXT("Failed to get Last Error, I know..."));
	}


}

void DirectX12Render::DirectX12Render::SyncToGPU()
{
	//This is bad. Simple and probably bad for later on
	const UINT64 fence = FenceValue;
	ThrowFail(CommandQueue->Signal(Fence.Get(), fence), TEXT("Failed to Signal Command queue"));
	FenceValue++;

	if (Fence->GetCompletedValue() < fence)
	{
		ThrowFail(Fence->SetEventOnCompletion(fence, FenceHandle), TEXT("Failed to set event on completion"));
		WaitForSingleObject(FenceHandle, INFINITE);
	}

	FenceIndex = SwapChain->GetCurrentBackBufferIndex();
}
