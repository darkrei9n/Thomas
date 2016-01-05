#pragma once
#include "stdafx.h"
#include "ExceptionHandler.h"

using namespace Microsoft::WRL;
using namespace Exceptions;

namespace DirectX12Render
{
	class DirectX12Render
	{
	public:
		//Updates the current frame and renders it
		void Update();
		//Public function for initializing DirectX
		void Initialize();

	private:
		static const UINT FrameCount = 2;

		//DirectX12 Devices need to initialize
		ComPtr<ID3D12Device>			Device;
		ComPtr<ID3D12CommandAllocator>	CommandAllocator;
		ComPtr<ID3D12CommandList>		CommandList;
		ComPtr<ID3D12CommandQueue>		CommandQueue;
		ComPtr<IDXGISwapChain3>			SwapChain;
		ComPtr<ID3D12Resource>			RenderTargets[FrameCount];
		ComPtr<ID3D12RootSignature>		RootSignature;
		ComPtr<ID3D12PipelineState>		Pipeline;
		ComPtr<ID3D12DescriptorHeap>	RTVHeap;

		//Fence stuff
		ComPtr<ID3D12Fence>	Fence;
		HANDLE				FenceHandle;
		UINT				FenceIndex;
		UINT				FenceValue;

		//Functions to initiate DirectX and run
		void InitializeDX();
		void InitializeAssets();
		void SyncToGPU();

	};
}