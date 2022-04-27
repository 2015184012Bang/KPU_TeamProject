#pragma once

#include <rapidjson/document.h>

#include "Resource.h"
#include "Vertex.h"

extern ComPtr<ID3D12Device> gDevice;
extern ComPtr<ID3D12GraphicsCommandList> gCmdList;
extern vector<ComPtr<ID3D12Resource>> gUsedUploadBuffers;

class Mesh : public IResource
{
	enum class eMeshType
	{
		Static,
		Skeletal
	};

public:
	Mesh();

	virtual void Load(const string& path) override;
	void LoadDebugMesh(const Vector3& minPoint, const Vector3& maxPoint);

	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return mVertexBufferView; }
	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return mIndexBufferView; }

	uint32 GetVertexCount() const { return mVertexCount; }
	uint32 GetIndexCount() const { return mIndexCount; }

private:
	rapidjson::Document openMeshFile(const string& path, eMeshType* outMeshType);
	void loadStaticMesh(const rapidjson::Document& doc);
	void loadSkeletalMesh(const rapidjson::Document& doc);

	void createIndexBuffer(const vector<uint32>& indices);

	template<typename T>
	void createVertexBuffer(const vector<T>& vertices)
	{
		if (vertices.empty())
		{
			HB_ASSERT(false, "Empty vertex vector");
		}

		mVertexCount = static_cast<uint32>(vertices.size());
		uint32 vertexBufferSize = mVertexCount * sizeof(T);

		ComPtr<ID3D12Resource> vertexUploadBuffer;
		const CD3DX12_HEAP_PROPERTIES uploadBufferHeapProps(D3D12_HEAP_TYPE_UPLOAD);
		const CD3DX12_RESOURCE_DESC uploadbufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		gDevice->CreateCommittedResource(
			&uploadBufferHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&uploadbufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertexUploadBuffer));

		void* mappedData;
		vertexUploadBuffer->Map(0, nullptr, &mappedData);
		memcpy(mappedData, vertices.data(), vertexBufferSize);
		vertexUploadBuffer->Unmap(0, nullptr);

		const CD3DX12_HEAP_PROPERTIES defaultBufferHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		const CD3DX12_RESOURCE_DESC defaultbufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		gDevice->CreateCommittedResource(
			&defaultBufferHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&defaultbufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&mVertexBuffer));

		gCmdList->CopyResource(mVertexBuffer.Get(), vertexUploadBuffer.Get());

		const auto toDefaultBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		gCmdList->ResourceBarrier(1, &toDefaultBarrier);

		gUsedUploadBuffers.push_back(vertexUploadBuffer);

		mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
		mVertexBufferView.SizeInBytes = vertexBufferSize;
		mVertexBufferView.StrideInBytes = sizeof(T);
	}

private:
	ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	uint32 mVertexCount;

	ComPtr<ID3D12Resource> mIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
	uint32 mIndexCount;
};

class SpriteMesh
{
public:
	SpriteMesh(int width, int height);

	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return mVertexBufferView; }
	uint32 GetVertexCount() const { return mVertexCount; }

private:
	void createVertexBuffer(int width, int height);

private:
	ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	uint32 mVertexCount;
};