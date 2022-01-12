#pragma once

#include "Resource.h"
#include "Vertex.h"

class Mesh : public IResource
{
public:
	Mesh();

	virtual void Load(const wstring& path) override;

	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return mVertexBufferView; }
	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return mIndexBufferView; }

	uint32 GetVertexCount() const { return mVertexCount; }
	uint32 GetIndexCount() const { return mIndexCount; }

private:
	bool createVertexBuffer(const vector<Vertex>& vertices);
	bool createIndexBuffer(const vector<uint32>& indices);

private:
	ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	uint32 mVertexCount;

	ComPtr<ID3D12Resource> mIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
	uint32 mIndexCount;
};

