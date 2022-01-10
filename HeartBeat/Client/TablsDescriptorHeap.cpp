#include "ClientPCH.h"
#include "TablsDescriptorHeap.h"

TablsDescriptorHeap::TablsDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 100;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	mSrvDescriptorSize = gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mDescriptorHeap));
}

D3D12_CPU_DESCRIPTOR_HANDLE TablsDescriptorHeap::GetCpuHandle(int index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), index * mSrvDescriptorSize);
}

D3D12_GPU_DESCRIPTOR_HANDLE TablsDescriptorHeap::GetGpuHandle(int index)
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(mDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), index * mSrvDescriptorSize);
}
