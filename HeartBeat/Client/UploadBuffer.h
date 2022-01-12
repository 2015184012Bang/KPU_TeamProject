#pragma once

inline uint32 CalculateConstantBufferByteSize(uint32 byteSize)
{
	return (byteSize + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
}

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* device, uint32 elementCount, bool isConstantBuffer) 
		: mbIsConstantBuffer(isConstantBuffer)
	{
		mElementByteSize = sizeof(T);

		if (isConstantBuffer)
			mElementByteSize = CalculateConstantBufferByteSize(sizeof(T));

		const CD3DX12_HEAP_PROPERTIES uploadBufferHeapProps(D3D12_HEAP_TYPE_UPLOAD);
		const CD3DX12_RESOURCE_DESC uploadbufferDesc = CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount);
		ThrowIfFailed(device->CreateCommittedResource(
			&uploadBufferHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&uploadbufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mUploadBuffer)));

		ThrowIfFailed(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));
	}

	~UploadBuffer()
	{
		if (mUploadBuffer != nullptr)
		{
			mUploadBuffer->Unmap(0, nullptr);
		}

		mMappedData = nullptr;
	}

	ID3D12Resource* Resource() const
	{
		return mUploadBuffer.Get();
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress() const
	{
		return mUploadBuffer->GetGPUVirtualAddress();
	}

	void CopyData(int elementIndex, const T& data)
	{
		memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
	}

private:
	ComPtr<ID3D12Resource> mUploadBuffer;
	BYTE* mMappedData = nullptr;

	uint32 mElementByteSize = 0;
	bool mbIsConstantBuffer = false;
};