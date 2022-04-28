#pragma once

#include "Protocol.h"
#include "Box.h"

constexpr UINT32 DATA_BUFFER_SIZE = 8192;
constexpr float PLAYER_MAX_SPEED = 300.0f;

class User
{
	const float BASE_ATTACK_COOLDOWN = 1.0f; // 기본 공격 재사용 대기시간

public:
	enum class UpgradePreset : UINT8
	{
		ATTACK = 0,
		HEAL = 1,
		SUPPORT = 2
	};

	User() = default;
	~User();

	void Init(const INT32 index);

	void Reset();

	void SetLogin(string_view userName);

	void SetData(const UINT32 dataSize, char* pData);

	PACKET_INFO GetPacket();

	void Update();

	void SetUpgrade(UpgradePreset preset);

	bool CanAttack();

public:
	INT32 GetIndex() const { return mIndex; }
	string GetUserName() const { return mUserName; }
	bool IsConnected() const { return mConnected; }
	Vector3 GetPosition() const { return mPosition; }
	float GetYaw() const { return mYaw; }
	Vector3 GetMoveDirection() const { return mMoveDirection; }
	INT32 GetBaseAttackDmg() const { return mBaseAttackDmg; }
	INT32 GetArmor() const { return mArmor; }
	INT32 GetRegeneration() const { return mRegeneration; }
	const Box& GetBox() { return mWorldBox; }

	void SetPosition(const Vector3& position) { mPosition = position; }
	void SetYaw(float yaw) { mYaw = yaw; }
	void SetMoveDirection(const Vector3& direction);
	void SetBaseAttackDmg(INT32 dmg) { mBaseAttackDmg = dmg; }
	void SetArmor(INT32 armor) { mArmor = armor; }
	void SetRegeneration(INT32 regen) { mRegeneration = regen; }

private:
	INT32 mIndex = -1;
	string mUserName = "";

	bool mConnected = false;

	UINT32 mWritePos = 0;
	UINT32 mReadPos = 0;
	char* mDataBuffer = nullptr;

	Vector3 mPosition = Vector3::Zero;
	float mYaw = 180.0f;
	Vector3 mMoveDirection = Vector3::Zero;

	INT32 mBaseAttackDmg = 0; // 기본 공격 데미지
	INT32 mArmor = 0; // 방어력
	INT32 mRegeneration = 0; // 회복력

	float mBaseAttackCooldown = BASE_ATTACK_COOLDOWN; // 기본 공격 대기시간 추적

	// 충돌 박스
	const Box* mLocalBox = nullptr;
	Box mWorldBox = {};
};

