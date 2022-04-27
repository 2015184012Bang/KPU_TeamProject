#pragma once

#include "fmod/fmod.hpp"

class SoundManager
{
public:
	static void Init();
	static void Shutdown();

	static void CreateSound(string_view fileName, bool bLoop = false);
	static void PlaySound(string_view sound, float volume = 1.0f);
	static void StopSound(string_view sound);

private:
	static void loadAllSounds();

private:
	static unordered_map<string, FMOD::Sound*> sSounds;
	static unordered_map<FMOD::Sound*, FMOD::Channel*> sChannels;
	static FMOD::System* sSystem;
};

