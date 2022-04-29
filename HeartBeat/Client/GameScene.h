#pragma once

#include "Scene.h"
#include "../IOCPServer/Protocol.h"
#include "Define.h"
#include "GameMap.h"

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
    void createMap();
    void createTile(const Tile& tile);
    void createBlockedTile(const Tile& tile);
    void createMovableTile(const Tile& tile);
    void createRailTile(const Tile& tile);
    void createFatTile(const Tile& tile);
    void createTankFatTile(const Tile& tile);
    void createScarTile(const Tile& tile);

	bool pollKeyboardPressed();
	bool pollKeyboardReleased();

	void processNotifyMove(const PACKET& packet);
    void processNotifyAttack(const PACKET& packet);

private:
    Entity mPlayerCharacter = {};
    Vector3 mDirection = Vector3::Zero;

    bool mbChangeScene = false;
};

string GetRandomAttackAnimFile(bool isEnemy = false);
Texture* GetTileTexture(TileType ttype);
