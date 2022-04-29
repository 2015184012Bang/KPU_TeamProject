#pragma once

extern float gTileWidth;
extern float gPlayerSpeed;

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
	END
};

enum class RootParameter
{
	WORLD_PARAM,
	VIEWPROJ_PARAM,
	TEX_PARAM,
	BONE_PARAM,
	END
};

enum class ShaderRegister
{
	B0,
	B1,
	B2,
	END
};