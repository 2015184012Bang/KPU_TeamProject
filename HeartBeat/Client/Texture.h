#pragma once

class Texture
{
public:
	Texture();

	void Load(const wstring& path);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() { return mSrvGpuHandle; }

private:
	bool loadTextureFromFile(const wstring& path);
	bool createShaderResource();

private:
	static UINT sNumTextures;

	UINT mTexIndex;
	ScratchImage mRawImage;
	ComPtr<ID3D12Resource> mTexture;
	D3D12_GPU_DESCRIPTOR_HANDLE mSrvGpuHandle;
};

