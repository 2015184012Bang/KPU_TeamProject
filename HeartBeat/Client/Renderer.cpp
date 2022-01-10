#include "ClientPCH.h"
#include "Renderer.h"

#include "Application.h"

#include "Vertex.h"

Renderer::Renderer()
	: mAspectRatio(0.0f)
	, mBackBufferIndex(0)
	, mRtvDescriptorSize(0)
	, mFenceValue(0)
	, mFenceEvent(nullptr)
{
	int width = Application::GetScreenWidth();
	int height = Application::GetScreenHeight();

	mAspectRatio = static_cast<float>(width) / static_cast<float>(height);

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
	ThrowIfFailed(mCmdList->Reset(mCmdAllocator.Get(), mPSO.Get()));

	mCmdList->SetGraphicsRootSignature(mRootSignature.Get());
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

	mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	mCmdList->DrawInstanced(3, 1, 0, 0);
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
	createPipelineState();
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

void Renderer::createPipelineState()
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputDesc;
	inputDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	inputDesc.push_back({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

#ifdef _DEBUG
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ThrowIfFailed(D3DCompileFromFile(L"default.hlsli", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(L"default.hlsli", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputDesc.data(), static_cast<UINT>(inputDesc.size()) };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}

void Renderer::createCmdList()
{
	ThrowIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		mCmdAllocator.Get(), nullptr, IID_PPV_ARGS(&mCmdList)));
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

void Renderer::createTestTriangle()
{
	float aspectRatio = mAspectRatio;

	{
		Vertex triangleVertices[] =
		{
			{ { 0.0f, 0.25f * aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);

		const auto hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		const auto rd = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		ThrowIfFailed(mDevice->CreateCommittedResource(
			&hp,
			D3D12_HEAP_FLAG_NONE,
			&rd,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mVertexBuffer)));

		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(mVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		mVertexBuffer->Unmap(0, nullptr);

		mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
		mVertexBufferView.StrideInBytes = sizeof(Vertex);
		mVertexBufferView.SizeInBytes = vertexBufferSize;
	}
}

void Renderer::loadAssets()
{
	// TODO :: load all assets
	createTestTriangle();
	//////////////////////////////////////////////////////////////////////////

	ThrowIfFailed(mCmdList->Close());
	ID3D12CommandList* cmdLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	waitForPreviousFrame();
}
