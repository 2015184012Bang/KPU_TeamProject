#pragma once

#include "Scene.h"
#include "../IOCPServer/Protocol.h"
#include "Define.h"
#include "GameMap.h"
#include "Entity.h"

class Texture;

class GameScene :
    public Scene
{
public:
    GameScene(Client* owner);

    virtual void Enter() override;
    virtual void Exit() override;
    virtual void ProcessInput() override;
    virtual void Update(float deltaTime) override;

    // 업그레이드 씬에서 넘어올 때 방향 동기화를 위해 필요하다.
    void SetDirection(const Vector3& direction) { mDirection = direction; }

private:
    void createMap(string_view mapFile);
    void createTile(const Tile& tile);
    void createBlockedTile(const Tile& tile);
    void createMovableTile(const Tile& tile);
    void createRailTile(const Tile& tile);
    void createFatTile(const Tile& tile);
    void createTankFatTile(const Tile& tile);
    void createScarTile(const Tile& tile);
    void createHouseTile(const Tile& tile);

    void createUI();

	bool pollKeyboardPressed();
	bool pollKeyboardReleased();

	void processNotifyMove(const PACKET& packet);

    // 플레이어 공격 패킷 처리
    void processNotifyAttack(const PACKET& packet);

    // Enemy 공격 패킷 처리
    void processNotifyEnemyAttack(const PACKET& packet);

    void processNotifyDeleteEntity(const PACKET& packet);
    void processNotifyCreateEntity(const PACKET& packet);
    void processNotifyGameOver(const PACKET& packet);
    void processNotifySkill(const PACKET& packet);
    void processNotifyStateChange(const PACKET& packet);

    void doGameOver();

private:
    enum class StageCode
    {
        NONE,
        CLEAR,
        GAMEOVER,
    };

    Entity mPlayerCharacter = {};
    Vector3 mDirection = Vector3::Zero;

    bool mbChangeScene = false;
    StageCode mStageCode = StageCode::NONE;

    Entity mO2Text = {};
    Entity mCO2Text = {};
};

string GetAttackAnimTrigger(bool isEnemy = false);
string GetSkillAnimTrigger(const uint8 preset);
string GetSkillSound(const uint8 preset);
Texture* GetTileTexture(TileType ttype);
