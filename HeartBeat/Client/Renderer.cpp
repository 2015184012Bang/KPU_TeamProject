#include "ClientPCH.h"
#include "Renderer.h"

#include "Application.h"

Renderer::Renderer()
	: mBackBufferIndex(0)
	, mRtvDescriptorSize(0)
	, mFenceValue(0)
	, mFenceEvent(nullptr)
{
	int width = Application::GetScreenWidth();
	int height = Application::GetScreenHeight();

	mViewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
		static_cast<float>(width), static_cast<float>(height));

	mScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
}

void Renderer::Init()
{
	ThrowIfFailed(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED));

	loadPipeline();
}

void Renderer::Shutdown()
{
	waitForPreviousFrame();

	CloseHandle(mFenceEvent);
}

void Renderer::BeginRender()
{
	ThrowIfFailed(mCmdAllocator->Reset());
	ThrowIfFailed(mCmdList->Reset(mCmdAllocator.Get(), nullptr));

	mCmdList->RSSetViewports(1, &mViewport);
	mCmdList->RSSetScissorRects(1, &mScissorRect);

	const auto toRenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mBackBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mCmdList->ResourceBarrier(1, &toRenderTargetBarrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE renderTagetView(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mBackBufferIndex, mRtvDescriptorSize);
	mCmdList->ClearRenderTargetView(renderTagetView, Colors::CornflowerBlue, 0, nullptr);

	mCmdList->OMSetRenderTargets(1, &renderTagetView, FALSE, nullptr);

	//D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	//mCmdList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	//mCmdList->OMSetRenderTargets(1, &renderTagetView, FALSE, &depthStencilView);
}

void Renderer::EndRender()
{
	const auto toPresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mBackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCmdList->ResourceBarrier(1, &toPresentBarrier);

	ThrowIfFailed(mCmdList->Close());

	ID3D12CommandList* cmdLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	ThrowIfFailed(mSwapChain->Present(1, 0));

	waitForPreviousFrame();
}

void Renderer::loadPipeline()
{
	createDevice();
	createCmdQueueAndSwapChain();
	createRtvHeap();
	createCmdAllocator();
	//CreateDsvHeap();
	createCmdList();
	createFence();

	loadAssets();
}

void Renderer::createDevice()
{
	UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif // _DEBUG

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory)));
	ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice)));

	gDevice = mDevice;
}

void Renderer::createCmdQueueAndSwapChain()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCmdQueue)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = BUFFER_COUNT;
	swapChainDesc.Width = Application::GetScreenWidth();
	swapChainDesc.Height = Application::GetScreenHeight();
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(mFactory->CreateSwapChainForHwnd(
		mCmdQueue.Get(),
		Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	ThrowIfFailed(mFactory->MakeWindowAssociation(Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));
	ThrowIfFailed(swapChain.As(&mSwapChain));

	mBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
}

void Renderer::createRtvHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = BUFFER_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));

	mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < BUFFER_COUNT; ++i)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])));
		mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, mRtvDescriptorSize);
	}
}

void Renderer::createCmdAllocator()
{
	ThrowIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCmdAllocator)));
}

void Renderer::createDsvHeap()
{
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Application::GetScreenWidth(), Application::GetScreenHeight());
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	const auto clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);
	mDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&mDsvBuffer));

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mDsvHeap));
	mDevice->CreateDepthStencilView(mDsvBuffer.Get(), nullptr, mDsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void Renderer::createCmdList()
{
	ThrowIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		mCmdAllocator.Get(), nullptr, IID_PPV_ARGS(&mCmdList)));

	// BEWARE!! CommandList is now recording state for loading assets.
}

void Renderer::createFence()
{
	ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
	mFenceValue = 1;

	mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (mFenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}

void Renderer::waitForPreviousFrame()
{
	const UINT64 fenceValue = mFenceValue;

	ThrowIfFailed(mCmdQueue->Signal(mFence.Get(), fenceValue));
	mFenceValue++;

	if (mFence->GetCompletedValue() < fenceValue)
	{
		ThrowIfFailed(mFence->SetEventOnCompletion(fenceValue, mFenceEvent));
		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	mBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
}

void Renderer::loadAssets()
{
	// TODO :: load all assets

	ThrowIfFailed(mCmdList->Close());
	ID3D12CommandList* cmdLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	waitForPreviousFrame();
}
