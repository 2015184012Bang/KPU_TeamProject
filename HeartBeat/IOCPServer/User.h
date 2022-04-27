#pragma once

#include "Protocol.h"

constexpr UINT32 DATA_BUFFER_SIZE = 8192;
constexpr float PLAYER_MAX_SPEED = 300.0f;

class User
{
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

	// TODO: 플레이어 정보 추가(ex. 체력 등)
	Vector3 mPosition = Vector3::Zero;
	float mYaw = 0.0f;
	Vector3 mMoveDirection = Vector3::Zero;

	INT32 mBaseAttackDmg = 0; // 기본 공격 데미지
	INT32 mArmor = 0; // 방어력
	INT32 mRegeneration = 0; // 회복력
};

