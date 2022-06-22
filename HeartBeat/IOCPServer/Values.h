#pragma once

class Values
{
public:
	static void Init();

	static UINT16 ServerPort;
	static float TileSide;
	static UINT8 TileMaxHealth;
	static float PlayerSpeed;
	static UINT8 PlayerHealth;
	static UINT8 PlayerLife;
	static float BaseAttackCooldown;
	static UINT8 BaseAttackPower;
	static float BaseAttackRange;
	static float SkillSlashCooldown;
	static float SkillHealCooldown;
	static float SkillBuffCooldown;
	static float SkillBuffDuration;
	static float TankSpeed;
	static UINT8 TankHealth;
	static float CellSpeed;
	static UINT8 CellPower;
	static UINT8 CellHealth;
	static float EnemySpeed;
	static UINT8 VirusHealth;
	static UINT8 VirusPower;
	static UINT8 DogHealth;
	static UINT8 DogPower;
	static UINT8 BossHealth;
	static UINT8 BossPower;
	static UINT8 ItemDrop;
	
	static UINT32 EntityID;
	static UINT8 MaxRoomNum;
};
