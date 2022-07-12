#pragma once

constexpr int MAX_TEX_DESCRIPTORS = 200;

class TableDescriptorHeap
{
public:
	TableDescriptorHeap();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(int index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(int index);

	ComPtr<ID3D12DescriptorHeap> GetHeap() { return mDescriptorHeap; }

private:
	uint32 mSrvDescriptorSize;

	ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;
};