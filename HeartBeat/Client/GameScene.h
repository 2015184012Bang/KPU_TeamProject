#pragma once

#include "Scene.h"
#include "../IOCPServer/Protocol.h"

class GameScene :
    public Scene
{
public:
    GameScene(Client* owner);

    virtual void Enter() override;
    virtual void Exit() override;
    virtual void ProcessInput() override;
    virtual void Update(float deltaTime) override;

    // ���׷��̵� ������ �Ѿ�� �� ���� ����ȭ�� ���� �ʿ��ϴ�.
    void SetDirection(const Vector3& direction) { mDirection = direction; }

private:
	bool pollKeyboardPressed();
	bool pollKeyboardReleased();

	void processAnswerMove(const PACKET& packet);
	void processNotifyMove(const PACKET& packet);

private:
    Entity mPlayerCharacter = {};
    Vector3 mDirection = Vector3::Zero;

    bool mbChangeScene = false;
};

string GetRandomAttackAnimFile();
