#include "ClientPCH.h"
#include "Texture.h"

#include "TableDescriptorHeap.h"
#include "Renderer.h"
#include "Utils.h"

uint32 Texture::sNumTextures = 0;

Texture::Texture()
	: mTexIndex(-1)
{

}

void Texture::Load(string_view path)
{
	bool success = loadTextureFromFile(path);

	if (!success)
	{
		HB_ASSERT(false, "Failed to load texture");
	}

	success = uploadTextureData();

	if (!success)
	{
		HB_ASSERT(false, "Failed to upload tex data to default buffer");
	}

	success = createSRV();

	if (!success)
	{
		HB_ASSERT(false, "Failed to create SRV");
	}
}

bool Texture::loadTextureFromFile(string_view path)
{
	auto ext = std::filesystem::path(path).extension();

	if (ext == ".dds" || ext == ".DDS")
	{
		LoadFromDDSFile(s2ws(path.data()).c_str(), DDS_FLAGS_NONE, nullptr, mRawImage);
	}
	else if (ext == ".tga" || ext == ".TGA")
	{
		LoadFromTGAFile(s2ws(path.data()).c_str(), nullptr, mRawImage);
	}
	else
	{
		LoadFromWICFile(s2ws(path.data()).c_str(), WIC_FLAGS_NONE, nullptr, mRawImage);
	}

	HRESULT hr = CreateTexture(gDevice.Get(), mRawImage.GetMetadata(), &mTexture);
	
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool Texture::uploadTextureData()
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

	const uint64 bufferSize = GetRequiredIntermediateSize(mTexture.Get(), 0, static_cast<uint32>(subResources.size()));

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

	const auto toShaderResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gCmdList->ResourceBarrier(1, &toShaderResourceBarrier);

	RELEASE_UPLOAD_BUFFER(textureUploadBuffer);

	return true;
}

bool Texture::createSRV()
{
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
