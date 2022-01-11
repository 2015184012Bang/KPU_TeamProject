#include "ClientPCH.h"
#include "Mesh.h"

void CreateTestBuffers(vector<Vertex>* outVertices, vector<uint32>* outIndices)
{
	*outVertices =
	{
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } },
		{ { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f } },
		{ { 0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f } },
		{ { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } },
	};
	
	*outIndices = { 0, 1, 2, 0, 2, 3};
}

Mesh::Mesh()
	: mVertexBufferView()
	, mVertexCount(0)
	, mIndexBufferView()
	, mIndexCount(0)
{

}

void Mesh::Load(const wstring& path)
{
	vector<Vertex> vertices;
	vector<uint32> indices;

	CreateTestBuffers(&vertices, &indices);

	bool success = createVertexBuffer(vertices);

	if (!success)
	{
		HB_LOG("Failed to create vertex buffer: {0}", ws2s(path));
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	success = createIndexBuffer(indices);

	if (!success)
	{
		HB_LOG("Failed to create index buffer: {0}", ws2s(path));
		HB_ASSERT(false, "ASSERTION FAILED");
	}
}

bool Mesh::createVertexBuffer(const vector<Vertex>& vertices)
{
	if (vertices.empty())
	{
		return false;
	}

	mVertexCount = static_cast<uint32>(vertices.size());
	uint32 vertexBufferSize = mVertexCount * sizeof(Vertex);

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

	RELEASE_UPLOAD_BUFFER(vertexUploadBuffer);

	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = vertexBufferSize;
	mVertexBufferView.StrideInBytes = sizeof(Vertex);

	return true;
}

bool Mesh::createIndexBuffer(const vector<uint32>& indices)
{
	if (indices.empty())
	{
		return false;
	}

	mIndexCount = static_cast<uint32>(indices.size());
	uint32 indexBufferSize = mIndexCount * sizeof(uint32);

	ComPtr<ID3D12Resource> indexUploadBuffer;
	const CD3DX12_HEAP_PROPERTIES uploadBufferHeapProps(D3D12_HEAP_TYPE_UPLOAD);
	const CD3DX12_RESOURCE_DESC uploadbufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
	gDevice->CreateCommittedResource(
		&uploadBufferHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadbufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&indexUploadBuffer));

	void* mappedData;
	indexUploadBuffer->Map(0, nullptr, &mappedData);
	memcpy(mappedData, indices.data(), indexBufferSize);
	indexUploadBuffer->Unmap(0, nullptr);

	const CD3DX12_HEAP_PROPERTIES defaultBufferHeapProps(D3D12_HEAP_TYPE_DEFAULT);
	const CD3DX12_RESOURCE_DESC defaultbufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
	gDevice->CreateCommittedResource(
		&defaultBufferHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&defaultbufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&mIndexBuffer));

	gCmdList->CopyResource(mIndexBuffer.Get(), indexUploadBuffer.Get());

	const auto toDefaultBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mIndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	gCmdList->ResourceBarrier(1, &toDefaultBarrier);
	
	RELEASE_UPLOAD_BUFFER(indexUploadBuffer);

	mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	mIndexBufferView.SizeInBytes = indexBufferSize;

	return true;
}
