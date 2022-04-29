#pragma once

constexpr int ENEMY_MAX_HEALTH = 3;
constexpr int PLAYER_MAX_HEALTH = 10;
constexpr float TILE_WIDTH = 400.0f;
constexpr float PLAYER_MAX_SPEED = 300.0f;

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