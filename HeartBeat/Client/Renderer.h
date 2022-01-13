#pragma once

class Mesh;
class Texture;

class Renderer
{
public:
	Renderer();

	void Init();
	void Shutdown();

	void BeginRender();
	void EndRender();
	void Submit(const Mesh* const mesh, const Texture* const texture);

private:
	void loadPipeline();
	void loadAssets();
	void loadAllAssetsFromFile();

	void createDevice();
	void createCmdQueueAndSwapChain();
	void createRtvHeap();
	void createCmdAllocator();
	void createDsvHeap();
	void createPipelineState();
	void createCmdList();
	void createFence();

	void waitForPreviousFrame();

private:
	static const int BUFFER_COUNT = 2;

	CD3DX12_VIEWPORT mViewport;
	CD3DX12_RECT mScissorRect;

	ComPtr<IDXGIFactory4> mFactory;
	ComPtr<ID3D12Device> mDevice;
	ComPtr<ID3D12CommandQueue> mCmdQueue;
	ComPtr<IDXGISwapChain3> mSwapChain;
	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	ComPtr<ID3D12Resource> mRenderTargets[BUFFER_COUNT];
	ComPtr<ID3D12CommandAllocator> mCmdAllocator;
	ComPtr<ID3D12Resource> mDsvBuffer;
	ComPtr<ID3D12DescriptorHeap> mDsvHeap;
	ComPtr<ID3D12GraphicsCommandList> mCmdList;
	ComPtr<ID3D12Fence> mFence;
	ComPtr<ID3D12RootSignature> mRootSignature;
	ComPtr<ID3D12PipelineState> mPSO;

	uint32 mBackBufferIndex;
	uint32 mRtvDescriptorSize;
	uint64 mFenceValue;

	HANDLE mFenceEvent;
};

