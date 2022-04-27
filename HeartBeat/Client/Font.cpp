#include "ClientPCH.h"
#include "Font.h"

Font::Font()
	: mTexture(nullptr)
{

}

void Font::Load(string_view path)
{
	loadFontFile(path);
}

void Font::MakeVertices(vector<SpriteVertex>* outVertices, const string& sentence) const
{
	int numLetters = static_cast<int>(sentence.size());

	if (numLetters == 0)
	{
		HB_LOG("Empty sentence!");
		return;
	}

	outVertices->clear();
	int numVertices = numLetters * 6;
	outVertices->reserve(numVertices);

	float drawX = 0.0f;
	float drawY = 0.0f;

	int letter = 0;
	for (int i = 0; i < numLetters; ++i)
	{
		letter = static_cast<int>(sentence[i]) - 32;

		if (letter == 0)
		{
			drawX += 3.0f;
		}
		else
		{
			outVertices->emplace_back(drawX, drawY, 0.0f, mChars[letter].Left, 0.0f); // Top-Left
			outVertices->emplace_back(drawX, (drawY - 16.0f), 0.0f, mChars[letter].Left, 1.0f); // Bottom-Left
			outVertices->emplace_back(drawX + mChars[letter].Size, (drawY - 16.0f), 0.0f, mChars[letter].Right, 1.0f); // Bottom-Right

			outVertices->emplace_back(drawX, drawY, 0.0f, mChars[letter].Left, 0.0f); // Top-Left
			outVertices->emplace_back(drawX + mChars[letter].Size, (drawY - 16.0f), 0.0f, mChars[letter].Right, 1.0f); // Bottom-Right
			outVertices->emplace_back(drawX + mChars[letter].Size, drawY, 0.0f, mChars[letter].Right, 0.0f); // Top-Right
		}

		drawX += mChars[letter].Size + 1.0f;
	}
}

void Font::loadFontFile(string_view path)
{
	std::ifstream file(path.data());

	if (!file.is_open())
	{
		HB_ASSERT(false, "Invalid file path.");
		return;
	}

	mChars.reserve(ASCII_CHAR_COUNT);

	char temp;
	for (int i = 0; i < ASCII_CHAR_COUNT; ++i)
	{
		file.get(temp);
		while (temp != ' ')
		{
			file.get(temp);
		}

		file.get(temp);

		while (temp != ' ')
		{
			file.get(temp);
		}

		FontType f;
		file >> f.Left;
		file >> f.Right;
		file >> f.Size;

		mChars.push_back(f);
	}
}
