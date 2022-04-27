#pragma once

#include "Scene.h"
#include "../IOCPServer/Protocol.h"

class UpgradeScene :
    public Scene
{
    const float DISTANCE_BETWEEN_PLANE = 700.0f;
    const float FIVE_SECS_BEFORE_START = 25.8f;
    const float SECS_TO_START = 30.0f;
	
    enum class UpgradePreset
	{
		ATTACK = 0,
		HEAL = 1,
		SUPPORT = 2,
	};

public:
    UpgradeScene(Client* owner);

    virtual void Enter() override;
    virtual void Exit() override;
    virtual void ProcessInput() override;
    virtual void Update(float deltaTime) override;

private:
    void initPlayersPositionToZero();
    void createPlanes();
    void createClock();
    void startCountdown();

    bool pollKeyboardPressed();
    bool pollKeyboardReleased();
    void checkCollisionWithPlanes();
    uint8 getPresetNumber(string_view planeName);
    void equipPresetToCharacter(Entity& target, UpgradePreset preset);
    void updateClockPosition();

    void processNotifyMove(const PACKET& packet);
    void processAnswerMove(const PACKET& packet);
    void processNotifyUpgrade(const PACKET& packet);
    void processNotifyEnterGame(const PACKET& packet);

private:
    Entity mPlayerCharacter = {};
    Entity mClock = {};

    Vector3 mDirection = Vector3::Zero;

    float mElapsed = 0.0f;
    bool mbCountdownPlayed = false;
    bool mbChangeScene = false;
};

