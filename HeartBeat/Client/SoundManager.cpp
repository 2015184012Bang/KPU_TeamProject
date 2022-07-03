#include "ClientPCH.h"
#include "SoundManager.h"

unordered_map<string, FMOD::Sound*> SoundManager::sSounds;
unordered_map<FMOD::Sound*, FMOD::Channel*> SoundManager::sChannels;
FMOD::System* SoundManager::sSystem;

void SoundManager::Init()
{
	FMOD_RESULT retVal = FMOD::System_Create(&sSystem);
	HB_ASSERT(retVal == FMOD_OK, "Failed to create fmod system.");

	void* extraData = nullptr;
	retVal = sSystem->init(32, FMOD_INIT_NORMAL, extraData);
	HB_ASSERT(retVal == FMOD_OK, "Failed to init fmod system");

	loadAllSounds();
}

void SoundManager::Shutdown()
{
	sSystem->release();
	sSystem = nullptr;
}

void SoundManager::CreateSound(string_view fileName, bool bLoop /*= false*/)
{
	auto sound = fs::path(fileName).filename();
	
	FMOD_MODE flag = bLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
	FMOD_RESULT retVal = sSystem->createSound(fileName.data(), flag, nullptr, &sSounds[sound.string()]);

	HB_ASSERT(retVal == FMOD_OK, "Failed to create sound: {0}", fileName.data());
}

void SoundManager::PlaySound(string_view sound, float volume /*= 1.0f*/)
{
	if (auto iter = sSounds.find(sound.data()); iter == sSounds.end())
	{
		HB_LOG("There is no sound: {0}", sound.data());
		return;
	}

	FMOD::Sound* soundPtr = sSounds[sound.data()];
	FMOD::Channel* channelPtr = sChannels[soundPtr];

	// 재생 이력이 있다면 channelPtr은 nullptr가 아니다.
	// 따라서 해당 Sound가 재생 중인지 확인해야 한다.
	if (channelPtr)
	{
		bool isPlaying = false;
		channelPtr->isPlaying(&isPlaying);

		if (isPlaying)
		{
			return;
		}
	}

	// 사운드 재생
	FMOD_RESULT retVal = sSystem->playSound(soundPtr, nullptr, false, &channelPtr);
	HB_ASSERT(retVal == FMOD_OK, "Failed to play sound: {0}", sound.data());

	// 볼륨 설정
	channelPtr->setVolume(volume);

	// 해쉬맵에 채널의 포인터 저장
	sChannels[soundPtr] = channelPtr;
}

void SoundManager::StopSound(string_view sound)
{
	if (auto iter = sSounds.find(sound.data()); iter == sSounds.end())
	{
		HB_LOG("There is no sound: {0}", sound.data());
		return;
	}

	FMOD::Sound* soundPtr = sSounds[sound.data()];
	FMOD::Channel* channelPtr = sChannels[soundPtr];

	// channelPtr가 nullptr라는 건 한 번도 재생하지
	// 않았던 사운드이므로 리턴한다.
	if (!channelPtr)
	{
		return;
	}

	bool isPlaying = false;
	channelPtr->isPlaying(&isPlaying);

	// 재생하고 있지 않다면 리턴
	if (!isPlaying)
	{
		return;
	}

	FMOD_RESULT retVal = channelPtr->stop();
	HB_ASSERT(retVal == FMOD_OK, "Failed to stop sound: {0}", sound.data());
}

void SoundManager::loadAllSounds()
{
	CreateSound("../Assets/Sounds/ClockTick.mp3");
	CreateSound("../Assets/Sounds/Countdown.mp3");
	CreateSound("../Assets/Sounds/Punch.mp3");
	CreateSound("../Assets/Sounds/NormalTheme.mp3", true);
	CreateSound("../Assets/Sounds/PlayerAttacked.mp3");
	CreateSound("../Assets/Sounds/Skill1.mp3");
	CreateSound("../Assets/Sounds/Skill2.mp3");
	CreateSound("../Assets/Sounds/Skill3.mp3");
	CreateSound("../Assets/Sounds/Warning.mp3");
	CreateSound("../Assets/Sounds/BattleTheme.mp3");
	CreateSound("../Assets/Sounds/ButtonClick.mp3");
	CreateSound("../Assets/Sounds/GameOver.mp3", true);
	CreateSound("../Assets/Sounds/MissileShot.mp3");
	CreateSound("../Assets/Sounds/VirusDead.mp3");
	CreateSound("../Assets/Sounds/DogDead.mp3");
	CreateSound("../Assets/Sounds/DogBomb.mp3");
	CreateSound("../Assets/Sounds/CellDead.mp3");
	CreateSound("../Assets/Sounds/ChangePreset.mp3");
	CreateSound("../Assets/Sounds/DoorOpen.mp3");
	CreateSound("../Assets/Sounds/GetVitamin.mp3");
	CreateSound("../Assets/Sounds/BossSpawn.mp3");
}