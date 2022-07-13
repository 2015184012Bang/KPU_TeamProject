#include "ClientPCH.h"
#include "Renderer.h"

#include "Define.h"

#include "Application.h"
#include "Mesh.h"
#include "ResourceManager.h"
#include "TableDescriptorHeap.h"
#include "Texture.h"
#include "Vertex.h"
#include "Utils.h"


ComPtr<ID3D12Device> gDevice;
ComPtr<ID3D12GraphicsCommandList> gCmdList;
vector<ComPtr<ID3D12Resource>> gUsedUploadBuffers;
TableDescriptorHeap* gTexDescHeap;

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

	if (gTexDescHeap)
	{
		delete gTexDescHeap;
		gTexDescHeap = nullptr;
	}

	ResourceManager::Shutdown();
}

void Renderer::BeginRender()
{
	ThrowIfFailed(mCmdAllocator->Reset());
	ThrowIfFailed(mCmdList->Reset(mCmdAllocator.Get(), nullptr));

	mCmdList->SetGraphicsRootSignature(mRootSignature.Get());
	mCmdList->RSSetViewports(1, &mViewport);
	mCmdList->RSSetScissorRects(1, &mScissorRect);

	const auto toRenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mBackBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mCmdList->ResourceBarrier(1, &toRenderTargetBarrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE renderTagetView(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mBackBufferIndex, mRtvDescriptorSize);
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = mDsvHeap->GetCPUDescriptorHandleForHeapStart();

	mCmdList->ClearRenderTargetView(renderTagetView, mBackgroundColor, 0, nullptr);
	mCmdList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	mCmdList->OMSetRenderTargets(1, &renderTagetView, FALSE, &depthStencilView);

	ID3D12DescriptorHeap* ppHeaps[] = { gTexDescHeap->GetHeap().Get() };
	mCmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

void Renderer::EndRender()
{
	const auto toPresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mBackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCmdList->ResourceBarrier(1, &toPresentBarrier);

	ThrowIfFailed(mCmdList->Close());

	ID3D12CommandList* cmdLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
}

void Renderer::loadPipeline()
{
	createDevice();
	createCmdQueueAndSwapChain();
	createRtvHeap();
	createCmdAllocator();
	createDsvHeap();
	createPipelineState();
	createCmdList();
	createFence();
	createD3D11onD12();

	loadAssets();
}

void Renderer::createDevice()
{
	uint32 dxgiFactoryFlags = 0;

#ifdef _DEBUG

	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
	}
#endif // _DEBUG

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory)));
	ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice)));

	gDevice = mDevice;
	gTexDescHeap = new TableDescriptorHeap;
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
	CD3DX12_DESCRIPTOR_RANGE descRange[1];
	descRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER params[static_cast<uint32>(RootParameter::END)];
	params[static_cast<uint32>(RootParameter::WORLD_PARAM)].InitAsConstantBufferView(static_cast<uint32>(ShaderRegister::B0), 0, D3D12_SHADER_VISIBILITY_VERTEX);
	params[static_cast<uint32>(RootParameter::VIEWPROJ_PARAM)].InitAsConstantBufferView(static_cast<uint32>(ShaderRegister::B1), 0, D3D12_SHADER_VISIBILITY_VERTEX);
	params[static_cast<uint32>(RootParameter::TEX_PARAM)].InitAsDescriptorTable(_countof(descRange), descRange, D3D12_SHADER_VISIBILITY_PIXEL);
	params[static_cast<uint32>(RootParameter::BONE_PARAM)].InitAsConstantBufferView(static_cast<uint32>(ShaderRegister::B2), 0, D3D12_SHADER_VISIBILITY_VERTEX);
	params[static_cast<uint32>(RootParameter::LIGHT_PARAM)].InitAsConstantBufferView(static_cast<uint32>(ShaderRegister::B3), 0, D3D12_SHADER_VISIBILITY_PIXEL);

	const auto samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(params),
		params, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

#ifdef _DEBUG
	uint32 compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	uint32 compileFlags = 0;
#endif

	// font
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DCompileFromFile(L"font.hlsli", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		hr = D3DCompileFromFile(L"font.hlsli", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
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

		ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mFontPSO)));
	}

	// PSO for sprite
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputDesc;
		inputDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DCompileFromFile(L"sprite.hlsli", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		hr = D3DCompileFromFile(L"sprite.hlsli", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputDesc.data(), static_cast<uint32>(inputDesc.size()) };
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
		psoDesc.BlendState.IndependentBlendEnable = FALSE;
		psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mSpritePSO)));
	}

	// PSO for static mesh
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputDesc;
		inputDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DCompileFromFile(L"static.hlsli", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		hr = D3DCompileFromFile(L"static.hlsli", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputDesc.data(), static_cast<uint32>(inputDesc.size()) };
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mStaticMeshPSO)));
	}

	// PSO for no lighting static mesh
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputDesc;
		inputDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DCompileFromFile(L"default.hlsli", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		hr = D3DCompileFromFile(L"default.hlsli", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputDesc.data(), static_cast<uint32>(inputDesc.size()) };
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mNoLightPSO)));
	}

	// PSO for background
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputDesc;
		inputDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DCompileFromFile(L"background.hlsli", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		hr = D3DCompileFromFile(L"background.hlsli", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputDesc.data(), static_cast<uint32>(inputDesc.size()) };
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mBackgroundPSO)));
	}

	// PSO for skeletal mesh
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputDesc;
		inputDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "BONE", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DCompileFromFile(L"skeletal.hlsli", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		hr = D3DCompileFromFile(L"skeletal.hlsli", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputDesc.data(), static_cast<uint32>(inputDesc.size()) };
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mSkeletalMeshPSO)));
	}

	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputDesc;
		inputDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> errorBlob;

		HRESULT hr = D3DCompileFromFile(L"wireframe.hlsli", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		hr = D3DCompileFromFile(L"wireframe.hlsli", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				HB_LOG((char*)errorBlob->GetBufferPointer());
				HB_ASSERT(false, "ASSERTION FAILED");
			}
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputDesc.data(), static_cast<uint32>(inputDesc.size()) };
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mWireframePSO)));
	}
}

void Renderer::createCmdList()
{
	ThrowIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		mCmdAllocator.Get(), nullptr, IID_PPV_ARGS(&mCmdList)));

	gCmdList = mCmdList;
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
	const uint64 fenceValue = mFenceValue;

	ThrowIfFailed(mCmdQueue->Signal(mFence.Get(), fenceValue));
	mFenceValue++;

	if (mFence->GetCompletedValue() < fenceValue)
	{
		ThrowIfFailed(mFence->SetEventOnCompletion(fenceValue, mFenceEvent));
		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	mBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
}

void Renderer::createD3D11onD12()
{
	UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	ComPtr<ID3D11Device> d3d11Device;
	ThrowIfFailed(D3D11On12CreateDevice(
		mDevice.Get(),
		d3d11DeviceFlags,
		nullptr,
		0,
		reinterpret_cast<IUnknown**>(mCmdQueue.GetAddressOf()),
		1,
		0,
		&d3d11Device,
		&mD3D11DeviceContext,
		nullptr
	));

	ThrowIfFailed(d3d11Device.As(&mD3D11On12Device));

	{
		D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};
		D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
		ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &mD2DFactory));
		ComPtr<IDXGIDevice> dxgiDevice;
		ThrowIfFailed(mD3D11On12Device.As(&dxgiDevice));
		ThrowIfFailed(mD2DFactory->CreateDevice(dxgiDevice.Get(), &mD2DDevice));
		ThrowIfFailed(mD2DDevice->CreateDeviceContext(deviceOptions, &mD2DDeviceContext));
		ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &mWriteFactory));
	}

	float dpiX;
	float dpiY;
#pragma warning(push)
#pragma warning(disable : 4996) // GetDesktopDpi is deprecated.
	mD2DFactory->GetDesktopDpi(&dpiX, &dpiY);
#pragma warning(pop)
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpiX,
		dpiY
	);


	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (int i = 0; i < BUFFER_COUNT; ++i)
		{
			ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])));
			mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);

			D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
			ThrowIfFailed(mD3D11On12Device->CreateWrappedResource(
				mRenderTargets[i].Get(),
				&d3d11Flags,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT,
				IID_PPV_ARGS(&mWrappedBackBuffers[i])
			));

			ComPtr<IDXGISurface> surface;
			ThrowIfFailed(mWrappedBackBuffers[i].As(&surface));
			ThrowIfFailed(mD2DDeviceContext->CreateBitmapFromDxgiSurface(
				surface.Get(),
				&bitmapProperties,
				&mD2DRenderTargets[i]
			));

			rtvHandle.Offset(1, mRtvDescriptorSize);
		}
	}
}

void Renderer::loadAllAssetsFromFile()
{
	MESH("Bag.mesh");
	MESH("Bat.mesh");
	MESH("Belt_Green.mesh");
	MESH("Belt_Pink.mesh");
	MESH("Belt_Red.mesh");
	MESH("Boss.mesh");
	MESH("Cart.mesh");
	MESH("Caffeine.mesh");
	MESH("Cell.mesh");
	MESH("Character_Green.mesh");
	MESH("Character_Pink.mesh");
	MESH("Character_Red.mesh");
	MESH("Cotton_Swab.mesh");
	MESH("CO2.mesh");
	MESH("Cube.mesh");
	MESH("Dog.mesh");
	MESH("Door.mesh");
	MESH("Fat.mesh");
	MESH("Hammer.mesh");
	MESH("HealPack.mesh");
	MESH("House.mesh");
	MESH("O2.mesh");
	MESH("Pickax.mesh");
	MESH("Pill.mesh");
	MESH("Pill_Pack.mesh");
	MESH("Plane.mesh");
	MESH("Ringer.mesh");
	MESH("Syringe.mesh");
	MESH("Tank.mesh");
	MESH("Thermometer.mesh");
	MESH("Virus.mesh");
	MESH("Vitamin.mesh");
	MESH("Missile.mesh");
	MESH("Tail.mesh");
	MESH("Wall.mesh");
	MESH("Bomb.mesh");
	MESH("Skill_Effect_Sup.mesh");
	MESH("Skill_Effect_Heal.mesh");
	MESH("Skill_Effect_Atk.mesh");
	MESH("BWall.mesh");
	MESH("Attack_Point.mesh");
	MESH("Change_Effect.mesh");
	MESH("Attack_Effect.mesh");
	MESH("Sweep_Area.mesh");
	MESH("Boss_Attack_Effect.mesh");

	SKELETON("Bag.skel");
	SKELETON("Boss.skel");
	SKELETON("Cart.skel");
	SKELETON("Cell.skel");
	SKELETON("Character_Green.skel");
	SKELETON("Character_Pink.skel");
	SKELETON("Character_Red.skel");
	SKELETON("Door.skel");
	SKELETON("Dog.skel");
	SKELETON("Fat.skel");
	SKELETON("HealPack.skel");
	SKELETON("Tank.skel");
	SKELETON("Virus.skel");
	SKELETON("Tail.skel");
	SKELETON("Wall.skel");
	SKELETON("Bomb.skel");
	SKELETON("Skill_Effect_Sup.skel");
	SKELETON("Skill_Effect_Heal.skel");
	SKELETON("Skill_Effect_Atk.skel");
	SKELETON("BWall.skel");
	SKELETON("Change_Effect.skel");
	SKELETON("Attack_Effect.skel");
	SKELETON("Boss_Attack_Effect.skel");

	BOX("Boss.box");
	BOX("Cart.box");
	BOX("Cell.box");
	BOX("Character.box");
	BOX("Cube.box");
	BOX("Dog.box");
	BOX("Door.box");
	BOX("House.box");
	BOX("Pickax.box");
	BOX("Tank.box");
	BOX("Virus.box");
	BOX("Wall.box");
	BOX("Tail.box");

	TEXTURE("Login_Background.png");
	TEXTURE("Start_Button.png");
	TEXTURE("Next_Button.png");
	TEXTURE("Login_Button.png");
	TEXTURE("Login_Form.png");
	TEXTURE("Back_Button.png");
	TEXTURE("Hp.png");
	TEXTURE("Trophy.png");
	TEXTURE("Skill_1_Slash.png");
	TEXTURE("Skill_2_Heal.png");
	TEXTURE("Skill_3_Power.png");
	TEXTURE("Warning.png");
	TEXTURE("Dialogue1.png");
	TEXTURE("Dialogue2.png");
	TEXTURE("Dialogue3.png");
	TEXTURE("Dialogue4.png");
	TEXTURE("Dialogue5.png");
	TEXTURE("Dialogue6.png");
	TEXTURE("Dialogue7.png");
	TEXTURE("Dialogue8.png");
	TEXTURE("Dialogue9.png");
	TEXTURE("Dialogue10.png");
	TEXTURE("Heart.png");
	TEXTURE("Tank_Portrait.png");
	TEXTURE("Red.png");
	TEXTURE("Lobby_Background.png");
	TEXTURE("Lobby_Button.png");
	TEXTURE("Waiting.png");
	TEXTURE("Playing.png");
	TEXTURE("Empty.png");
	TEXTURE("Room_Background.png");
	TEXTURE("P1.png");
	TEXTURE("P2.png");
	TEXTURE("P3.png");
	TEXTURE("Player_Name.png");
	TEXTURE("Room_Message.png");
	TEXTURE("Game_Background.png");
	TEXTURE("Ready_Button.png");
	TEXTURE("Hpbar_Green_0.png");
	TEXTURE("Hpbar_Green_3.png");
	TEXTURE("Hpbar_Green_6.png");
	TEXTURE("Hpbar_Green_10.png");
	TEXTURE("Hpbar_Pink_0.png");
	TEXTURE("Hpbar_Pink_3.png");
	TEXTURE("Hpbar_Pink_6.png");
	TEXTURE("Hpbar_Pink_10.png");
	TEXTURE("Hpbar_Red_0.png");
	TEXTURE("Hpbar_Red_3.png");
	TEXTURE("Hpbar_Red_6.png");
	TEXTURE("Hpbar_Red_10.png");
	TEXTURE("Attacker_Board.png");
	TEXTURE("Healer_Board.png");
	TEXTURE("Supporter_Board.png");
	TEXTURE("Skill_Board.png");
	TEXTURE("Attacker_Explain.png");
	TEXTURE("Healer_Explain.png");
	TEXTURE("Supporter_Explain.png");
	TEXTURE("Select.png");
	TEXTURE("Skill_1_Explain.png");
	TEXTURE("Skill_2_Explain.png");
	TEXTURE("Skill_3_Explain.png");
	TEXTURE("Timer.png");
	TEXTURE("Upgrade_Background.png");
}

void Renderer::loadAssets()
{
	loadAllAssetsFromFile();
	loadFont(40.0f, D2D1::ColorF::Black, mTextFormat_40);
	loadFont(30.0f, D2D1::ColorF::Black, mTextFormat_30);

	ThrowIfFailed(mCmdList->Close());
	ID3D12CommandList* cmdLists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	waitForPreviousFrame();

	for (auto buffer : gUsedUploadBuffers)
	{
		buffer = nullptr;
	}
	gUsedUploadBuffers.clear();
}

void Renderer::Submit(const Mesh* mesh, const Texture* texture)
{
	mCmdList->SetGraphicsRootDescriptorTable(static_cast<uint32>(RootParameter::TEX_PARAM), texture->GetGpuHandle());
	mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCmdList->IASetVertexBuffers(0, 1, &mesh->GetVertexBufferView());
	mCmdList->IASetIndexBuffer(&mesh->GetIndexBufferView());
	mCmdList->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
}

void Renderer::SubmitDebugMesh(const Mesh* mesh)
{
	mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCmdList->IASetVertexBuffers(0, 1, &mesh->GetVertexBufferView());
	mCmdList->IASetIndexBuffer(&mesh->GetIndexBufferView());
	mCmdList->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
}

void Renderer::SubmitSprite(const SpriteMesh* mesh, const Texture* texture)
{
	mCmdList->SetGraphicsRootDescriptorTable(static_cast<uint32>(RootParameter::TEX_PARAM), texture->GetGpuHandle());
	mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCmdList->IASetVertexBuffers(0, 1, &mesh->GetVertexBufferView());
	mCmdList->DrawInstanced(mesh->GetVertexCount(), 1, 0, 0);
}

void Renderer::SetBackgroundColor(const XMVECTORF32& color)
{
	mBackgroundColor = color;
}

void Renderer::RenderFont(const vector<Sentence>& sentences)
{
	mCmdList->SetPipelineState(mFontPSO.Get());

	D2D1_SIZE_F rtSize = mD2DRenderTargets[mBackBufferIndex]->GetSize();

	mD3D11On12Device->AcquireWrappedResources(mWrappedBackBuffers[mBackBufferIndex].GetAddressOf(), 1);

	mD2DDeviceContext->SetTarget(mD2DRenderTargets[mBackBufferIndex].Get());
	mD2DDeviceContext->BeginDraw();
	mD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

	for (const auto& sent : sentences)
	{
		D2D1_RECT_F textRect = D2D1::RectF(sent.X, sent.Y, rtSize.width, rtSize.height);

		const auto& textFormat = sent.FontSize == 40 ? mTextFormat_40 : mTextFormat_30;

		mD2DDeviceContext->DrawText(
			sent.Text->data(),
			sent.TextLen,
			textFormat.Get(),
			&textRect,
			mTextBrush.Get()
		);
	}

	ThrowIfFailed(mD2DDeviceContext->EndDraw());

	mD3D11On12Device->ReleaseWrappedResources(mWrappedBackBuffers[mBackBufferIndex].GetAddressOf(), 1);

	mD3D11DeviceContext->Flush();
}

void Renderer::Present()
{
	ThrowIfFailed(mSwapChain->Present(1, 0));
	waitForPreviousFrame();
}

void Renderer::loadFont(float fontSize, const D2D1::ColorF fontColor, ComPtr<IDWriteTextFormat>& font)
{
	ThrowIfFailed(mD2DDeviceContext->CreateSolidColorBrush(fontColor, &mTextBrush));
	ThrowIfFailed(mWriteFactory->CreateTextFormat(
		L"ÈÞ¸ÕµÕ±ÙÇìµå¶óÀÎ",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fontSize,
		L"en-us",
		&font
	));
	ThrowIfFailed(font->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
	ThrowIfFailed(font->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
}