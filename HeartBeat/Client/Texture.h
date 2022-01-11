#pragma once

#include "Resource.h"

class Texture : public IResource
{
public:
	Texture();

	virtual void Load(const wstring& path) override;

	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() { return mSrvGpuHandle; }

private:
	bool loadTextureFromFile(const wstring& path);
	bool uploadTextureData();
	bool createSRV();

private:
	static uint32 sNumTextures;

	uint32 mTexIndex;
	ScratchImage mRawImage;
	ComPtr<ID3D12Resource> mTexture;
	D3D12_GPU_DESCRIPTOR_HANDLE mSrvGpuHandle;
};

