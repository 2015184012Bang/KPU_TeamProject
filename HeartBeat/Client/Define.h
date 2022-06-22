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

	// 클라이언트의 경우 START_POINT 타일과 END_POINT 타일은
	// 그냥 RAIL 타일 취급하면 된다. 서버와 맵 파일을 공유하기 위해서 넣었다.
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
