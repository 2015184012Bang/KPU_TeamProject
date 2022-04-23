#include "ClientPCH.h"
#include "TableDescriptorHeap.h"

#include "Renderer.h"

TableDescriptorHeap::TableDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = MAX_TEX_DESCRIPTORS;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	mSrvDescriptorSize = gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mDescriptorHeap));
}

D3D12_CPU_DESCRIPTOR_HANDLE TableDescriptorHeap::GetCpuHandle(int index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), index * mSrvDescriptorSize);
}

D3D12_GPU_DESCRIPTOR_HANDLE TableDescriptorHeap::GetGpuHandle(int index)
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), index * mSrvDescriptorSize);
}
