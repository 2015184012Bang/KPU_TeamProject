#pragma once



enum class EnemyType: uint8
{
	VIRUS,
	DOG,
};

enum class TileType : uint8
{
	BLOCKED = 0,
	MOVABLE,
	RAIL,
	FAT,
	TANK_FAT,
	SCAR,

	// Ŭ���̾�Ʈ�� ��� START_POINT Ÿ�ϰ� END_POINT Ÿ����
	// �׳� RAIL Ÿ�� ����ϸ� �ȴ�. ������ �� ������ �����ϱ� ���ؼ� �־���.
	START_POINT,
	END_POINT,
	HOUSE,
	SCAR_DOG,
	MID_POINT = 10,
	DOOR = 11,
	BATTLE_TRIGGER = 12,
	SCAR_WALL = 13,
	BOSS_TRIGGER = 14,
	SCAR_BOSS = 15,
	END
};

enum class RootParameter
{
	WORLD_PARAM,
	VIEWPROJ_PARAM,
	TEX_PARAM,
	BONE_PARAM,
	LIGHT_PARAM,
	END
};

enum class ShaderRegister
{
	B0,
	B1,
	B2,
	B3,
	END
};

enum class UpgradePreset
{
	ATTACK = 0,
	HEAL = 1,
	SUPPORT = 2,
};

class Values
{
public:
	static void Init();

	static uint16 ServerPort;
	static string ServerIP;
	static float TileSide;
	static float PlayerSpeed;
	static float TankSpeed;
	static float EnemySpeed;
	static float CellSpeed;
	static uint32 EntityID;
	static int HostID;
};
