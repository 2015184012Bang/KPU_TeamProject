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
    friend class Timer;

public:
    GameScene(Client* owner);

    virtual void Enter() override;
    virtual void Exit() override;
    virtual void ProcessInput() override;
    virtual void Update(float deltaTime) override;

    // 업그레이드 씬에서 넘어올 때 방향 동기화를 위해 필요하다.
    void SetDirection(const Vector3& direction) { mDirection = direction; }

private:
    void updateLightPosition();
    void updateUI(float deltaTime);

    void createMap(string_view mapFile);
    void createTile(const Tile& tile);
    void createBlockedTile(const Tile& tile);
    void createMovableTile(const Tile& tile);
    void createRailTile(const Tile& tile);
    void createFatTile(const Tile& tile);
    void createTankFatTile(const Tile& tile);
    void createScarTile(const Tile& tile);
    void createHouseTile(const Tile& tile);
    void createDoorTile(const Tile& tile);
    void createWallTile(const Tile& tile);
    void createBossTile(const Tile& tile);

    void createUI();
    void createHpbar();
    void createAttackEffect(const UINT32 entityID);
    void createSkillEffect(const UINT32 entityID, const UINT8 preset);
    void createDialogue(Texture* dia, int drawOrder);
    void clearDialogue();
    void changeHitTexture(EntityType eType, const UINT32 entityID);

	bool pollKeyboardPressed();
	bool pollKeyboardReleased();

	void processNotifyMove(const PACKET& packet);
    void processNotifyAttack(const PACKET& packet);
    void processNotifyEnemyAttack(const PACKET& packet);
    void processNotifyDeleteEntity(const PACKET& packet);
    void processNotifyCreateEntity(const PACKET& packet);
    void processNotifyGameOver(const PACKET& packet);
    void processNotifySkill(const PACKET& packet);
    void processNotifyStateChange(const PACKET& packet);
    void processNotifyEventOccur(const PACKET& packet);

    void doGameOver();
    void doBattleOccur();
    void doBattleEnd();
    void doBossBattleOccur();
    void doBossBattleEnd();
    void doBossSkill(const UINT8 skillType);
    void createBossFakeWall(float timeSec);

    void updateHpUI(const INT8 hp, int clientID);
    void updatePlayerPortrait(const INT8 hp, int clientID);

    void doCameraShake(float timeSec, bool bShakeX = false);
    
private:
    enum class StageCode
    {
        NONE,
        CLEAR,
        GAMEOVER,
    };

    Entity mPlayerCharacter = {};
    Vector3 mDirection = Vector3::Zero;

    // UI things
    Entity mScoreText = {};
    Entity mTankHpText = {};
    Entity mCooldownText = {};
    float mCooldown = 0.0f;
    vector<vector<Entity>> mHps;

    bool mbIsGameOver = false;

    int mBossSpecialSkillUsageCount = 0;

    float mPlayTime = 0.0f;
    Entity mPlayTimeText = {};
};

string GetAttackAnimTrigger(bool isEnemy = false);
string GetSkillAnimTrigger(const uint8 preset);
string GetSkillSound(const uint8 preset);
float GetSkillWaitTime(const uint8 preset);
Texture* GetTileTexture(TileType ttype);
Texture* GetHpbarTexture(const int clientID);
Texture* GetSkillTexture(UpgradePreset preset);
float GetSkillCooldown(UpgradePreset preset);
void PlayHitSound(const uint8 entityType);