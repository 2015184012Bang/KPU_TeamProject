#pragma once

constexpr int ENEMY_MAX_HEALTH = 3;
constexpr int PLAYER_MAX_HEALTH = 10;
constexpr int SERVER_PORT = 9000;
constexpr float TILE_WIDTH = 400.0f;
constexpr float PLAYER_MAX_SPEED = 300.0f;

enum class eEnemyType: uint8
{
	Virus,
	Dog,
};

enum : uint8
{
	Grass,
	Rail,
	Obstacle,
};

enum class eRootParameter
{
	WorldParam,
	ViewProjParam,
	TexParam,
	BoneParam,
	End
};

enum class eShaderRegister
{
	B0,
	B1,
	B2,
	End
};