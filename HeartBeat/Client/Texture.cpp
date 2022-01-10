#include "ClientPCH.h"
#include "Texture.h"

#include "TablsDescriptorHeap.h"

UINT Texture::sNumTextures = 0;

Texture::Texture()
	: mTexIndex(-1)
{

}

void Texture::Load(const wstring& path)
{
	bool success = loadTextureFromFile(path);

	if (!success)
	{
		HB_LOG("Failed to load texture: {0}", ws2s(path));
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	success = createShaderResource();

	if (!success)
	{
		HB_LOG("Failed to create SRV: {0}", ws2s(path));
		HB_ASSERT(false, "ASSERTION FAILED");
	}
}

bool Texture::loadTextureFromFile(const wstring& path)
{
	std::wstring ext = std::filesystem::path(path).extension();

	if (ext == L".dds" || ext == L".DDS")
	{
		LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, nullptr, mRawImage);
	}
	else if (ext == L".tga" || ext == L".TGA")
	{
		LoadFromTGAFile(path.c_str(), nullptr, mRawImage);
	}
	else
	{
		LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, nullptr, mRawImage);
	}

	HRESULT hr = ::CreateTexture(gDevice.Get(), mRawImage.GetMetadata(), &mTexture);
	
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool Texture::createShaderResource()
{
	vector<D3D12_SUBRESOURCE_DATA> subResources;

	HRESULT hr = PrepareUpload(gDevice.Get(),
		mRawImage.GetImages(),
		mRawImage.GetImageCount(),
		mRawImage.GetMetadata(),
		subResources);

	if (FAILED(hr))
	{
		return false;
	}

	const UINT64 bufferSize = GetRequiredIntermediateSize(mTexture.Get(), 0, static_cast<UINT>(subResources.size()));

	D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	ComPtr<ID3D12Resource> textureUploadBuffer;
	hr = gDevice->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(textureUploadBuffer.GetAddressOf()));

	if (FAILED(hr))
	{
		return false;
	}

	UpdateSubresources(gCmdList.Get(),
		mTexture.Get(),
		textureUploadBuffer.Get(),
		0, 0,
		static_cast<unsigned int>(subResources.size()),
		subResources.data());

	const auto toSrvBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gCmdList->ResourceBarrier(1, &toSrvBarrier);

	RELEASE_UPLOAD_BUFFER(std::move(textureUploadBuffer));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = mRawImage.GetMetadata().format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	mTexIndex = sNumTextures;
	sNumTextures++;

	gDevice->CreateShaderResourceView(mTexture.Get(), &srvDesc, gTexDescHeap->GetCpuHandle(mTexIndex));

	mSrvGpuHandle = gTexDescHeap->GetGpuHandle(mTexIndex);
	return true;
}
