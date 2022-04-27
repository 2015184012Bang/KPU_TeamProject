#pragma once

#include "Resource.h"

class Texture : public IResource
{
public:
	Texture();

	virtual void Load(string_view path) override;

	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return mSrvGpuHandle; }

private:
	bool loadTextureFromFile(string_view path);
	bool uploadTextureData();
	bool createSRV();

private:
	static uint32 sNumTextures;

	uint32 mTexIndex;
	ScratchImage mRawImage;
	ComPtr<ID3D12Resource> mTexture;
	D3D12_GPU_DESCRIPTOR_HANDLE mSrvGpuHandle;
};

