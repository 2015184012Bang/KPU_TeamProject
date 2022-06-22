#pragma once

class Values
{
public:
	static void Init();

	static UINT16 ServerPort;
	static float TileSide;
	static float PlayerSpeed;
	static float BaseAttackCooldown;
	static float SkillCooldown;
	static float BaseAttackRange;
	static float TankSpeed;
	static float EnemySpeed;
	static float CellSpeed;
	static UINT8 TankHealth;
	static UINT8 EnemyHealth;
	static UINT8 PlayerHealth;
	static UINT8 CellHealth;
	static UINT32 EntityID;
	static UINT8 MaxRoomNum;
};
