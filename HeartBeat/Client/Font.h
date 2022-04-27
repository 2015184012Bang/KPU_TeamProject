#pragma once
#include "Resource.h"
#include "Vertex.h"

class Texture;

constexpr int ASCII_CHAR_COUNT = 95;

class Font :
	public IResource
{
	struct FontType
	{
		float Left;
		float Right;
		int Size;
	};

public:
	Font();

	virtual void Load(string_view path) override;
	void SetTexture(const Texture* texture) { mTexture = texture; }
	const Texture* GetTexture() const { HB_ASSERT(mTexture, "Texture not set!");  return mTexture; }

	void MakeVertices(vector<SpriteVertex>* outVertices, const string& sentence) const;

private:
	void loadFontFile(string_view path);

private:
	vector<FontType> mChars;
	const Texture* mTexture;
};

