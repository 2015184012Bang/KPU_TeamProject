#pragma once

#include "Font.h"

class Texture;

constexpr int TEXT_INPUT_LIMIT = 24;

class Text
{
public:
	Text();
	Text(const Font* font);

	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return mVertexBufferView; }
	uint32 GetVertexCount() const { return mVertexCount; }

	void SetFont(const Font* font) { mFont = font; }
	void SetSentence(string_view sentence);

	const Texture* GetTexture() const { return mFont->GetTexture(); }
	const string& GetSentence() const { return mSentence; }

private:
	void createVertexBuffer();
	void updateVertexBuffer();

private:
	const Font* mFont;

	ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	uint32 mVertexCount;

	string mSentence;

	void* mMappedData;
};

