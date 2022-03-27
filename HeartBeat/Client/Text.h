#pragma once

class Font;

class Text
{
public:
	Text(const Font* font, const string& sentence);

	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return mVertexBufferView; }
	uint32 GetVertexCount() const { return mVertexCount; }

	void SetSentence(const string& sentence);

	const Texture* GetTexture() const { return mFont->GetTexture(); }

private:
	void createVertexBuffer();

private:
	const Font* mFont;

	ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	uint32 mVertexCount;

	string mSentence;

	void* mMappedData;
};

