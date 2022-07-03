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

	// ��� �̷��� �ִٸ� channelPtr�� nullptr�� �ƴϴ�.
	// ���� �ش� Sound�� ��� ������ Ȯ���ؾ� �Ѵ�.
	if (channelPtr)
	{
		bool isPlaying = false;
		channelPtr->isPlaying(&isPlaying);

		if (isPlaying)
		{
			return;
		}
	}

	// ���� ���
	FMOD_RESULT retVal = sSystem->playSound(soundPtr, nullptr, false, &channelPtr);
	HB_ASSERT(retVal == FMOD_OK, "Failed to play sound: {0}", sound.data());

	// ���� ����
	channelPtr->setVolume(volume);

	// �ؽ��ʿ� ä���� ������ ����
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

	// channelPtr�� nullptr��� �� �� ���� �������
	// �ʾҴ� �����̹Ƿ� �����Ѵ�.
	if (!channelPtr)
	{
		return;
	}

	bool isPlaying = false;
	channelPtr->isPlaying(&isPlaying);

	// ����ϰ� ���� �ʴٸ� ����
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