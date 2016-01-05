#pragma once
#include "ThomasTheTankEngine.h"
#include "ExceptionHandler.h"

using namespace Microsoft::WRL;
using namespace Exceptions;


class DirectX12Render
{
public:
	DirectX12Render() { };
	//Updates the current frame and renders it
	virtual void Update();
	//Public function for initializing DirectX
	virtual void Initialize(UINT width, UINT Height);


private:
	static const UINT FrameCount = 2;
	//DirectX12 Devices need to initialize
	ComPtr<ID3D12Device>			Device;
	ComPtr<ID3D12CommandAllocator>	CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList>		CommandList;
	ComPtr<ID3D12CommandQueue>		CommandQueue;
	ComPtr<IDXGISwapChain3>			SwapChain;
	ComPtr<ID3D12Resource>			RenderTargets[FrameCount];
	ComPtr<ID3D12RootSignature>		RootSignature;
	ComPtr<ID3D12PipelineState>		Pipeline;
	ComPtr<ID3D12DescriptorHeap>	RTVHeap;
	UINT							HeapSize;

	//Fence stuff
	ComPtr<ID3D12Fence>	Fence;
	HANDLE				FenceHandle;
	UINT				FenceIndex;
	UINT				FenceValue;

	//Window Size
	UINT Height;
	UINT Width;

	//Functions to initiate DirectX and run
	void InitializeDX();
	void InitializeAssets();
	void SyncToGPU();

};
