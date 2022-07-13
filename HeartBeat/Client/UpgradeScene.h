#pragma once

#include "Scene.h"
#include "../IOCPServer/Protocol.h"

#include "Entity.h"
#include "Define.h"

class UpgradeScene :
    public Scene
{
public:
    UpgradeScene(Client* owner);

    virtual void Enter() override;
    virtual void Exit() override;
    virtual void Update(float deltaTime) override;
    virtual void ProcessInput() override;

private:
    void createUI();
	void createButtons();
    void createExplainUI(UpgradePreset preset);

	void createChangeEffect(const Vector3& pos);

    void equipPresetToCharacter(Entity& target, UpgradePreset preset);
    void processNotifyUpgrade(const PACKET& packet);
    void processNotifyEnterGame(const PACKET& packet);

private:
    Entity mPlayerCharacter = {};
    bool mbChangeScene = false;

    Entity mCountdownText = {};
    float mElapsedTime = 0.0f;
};

