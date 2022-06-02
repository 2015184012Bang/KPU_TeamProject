#pragma once

class Mesh;
class Texture;
class SpriteMesh;

extern ComPtr<ID3D12Device> gDevice;
extern ComPtr<ID3D12GraphicsCommandList> gCmdList;
extern vector<ComPtr<ID3D12Resource>> gUsedUploadBuffers;
extern class TableDescriptorHeap* gTexDescHeap;

#define RELEASE_UPLOAD_BUFFER(x) gUsedUploadBuffers.push_back(x)

struct Sentence
{
	Sentence() = default;

	Sentence(std::wstring* _text,
		UINT32 _textLen,
		float _x,
		float _y)
		: Text(_text)
		, TextLen(_textLen)
		, X(_x)
		, Y(_y) {}

	std::wstring* Text = nullptr;
	UINT32 TextLen = 0;
	float X = 0.0f;
	float Y = 0.0f;
};

class Renderer
{
public:
	Renderer();

	void Init();
	void Shutdown();

	void BeginRender();
	void EndRender();
	void RenderFont(const vector<Sentence>& sentences);
	void Present();

	void Submit(const Mesh* mesh, const Texture* texture);
	void SubmitDebugMesh(const Mesh* mesh);
	void SubmitSprite(const SpriteMesh* mesh, const Texture* texture);

	void SetBackgroundColor(const XMVECTORF32& color);

	const ComPtr<ID3D12PipelineState>& GetStaticMeshPSO() const { return mStaticMeshPSO; }
	const ComPtr<ID3D12PipelineState>& GetSkeletalMeshPSO() const { return mSkeletalMeshPSO; }
	const ComPtr<ID3D12PipelineState>& GetWireframePSO() const { return mWireframePSO; }
	const ComPtr<ID3D12PipelineState>& GetSpritePSO() const { return mSpritePSO; }
	const ComPtr<ID3D12PipelineState>& GetNoLightPSO() const { return mNoLightPSO; }
	const ComPtr<ID3D12PipelineState>& GetFontPSO() const { return mFontPSO; }

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
	void createD3D11onD12();

	void waitForPreviousFrame();

	void loadFont(float fontSize, const D2D1::ColorF fontColor);

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
	ComPtr<ID3D12PipelineState> mStaticMeshPSO;
	ComPtr<ID3D12PipelineState> mSkeletalMeshPSO;
	ComPtr<ID3D12PipelineState> mWireframePSO;
	ComPtr<ID3D12PipelineState> mSpritePSO;
	ComPtr<ID3D12PipelineState> mNoLightPSO;
	ComPtr<ID3D12PipelineState> mFontPSO;

	ComPtr<ID3D11DeviceContext> mD3D11DeviceContext;
	ComPtr<ID3D11On12Device> mD3D11On12Device;
	ComPtr<IDWriteFactory> mWriteFactory;
	ComPtr<ID2D1Factory3> mD2DFactory;
	ComPtr<ID2D1Device2> mD2DDevice;
	ComPtr<ID2D1DeviceContext2> mD2DDeviceContext;
	ComPtr<ID3D11Resource> mWrappedBackBuffers[BUFFER_COUNT];
	ComPtr<ID2D1Bitmap1> mD2DRenderTargets[BUFFER_COUNT];

	ComPtr<ID2D1SolidColorBrush> mTextBrush;
	ComPtr<IDWriteTextFormat> mTextFormat;

	uint32 mBackBufferIndex;
	uint32 mRtvDescriptorSize;
	uint64 mFenceValue;

	HANDLE mFenceEvent;

	XMVECTORF32 mBackgroundColor = {};
};

