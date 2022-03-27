#include "ClientPCH.h"
#include "Text.h"

#include "ClientSystems.h"

Text::Text(const Font* font, const string& sentence)
	: mFont(font)
	, mVertexBufferView()
	, mVertexCount(0)
	, mSentence(sentence)
{
	createVertexBuffer();
}

void Text::SetSentence(const string& sentence)
{
	mSentence = sentence;
}

void Text::createVertexBuffer()
{
	vector<SpriteVertex> vertices;
	mFont->MakeVertices(&vertices, mSentence);

	mVertexCount = vertices.size();

	uint32 vertexBufferSize =  mVertexCount * sizeof(SpriteVertex);

	const CD3DX12_HEAP_PROPERTIES uploadBufferHeapProps(D3D12_HEAP_TYPE_UPLOAD);
	const CD3DX12_RESOURCE_DESC uploadbufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	gDevice->CreateCommittedResource(
		&uploadBufferHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadbufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mVertexBuffer));

	mVertexBuffer->Map(0, nullptr, &mMappedData);
	memcpy(mMappedData, vertices.data(), vertexBufferSize);
	
	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = vertexBufferSize;
	mVertexBufferView.StrideInBytes = sizeof(SpriteVertex);
}
