#pragma once

#include "tinyxml2.h"

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
	static void Init()
	{
		string fileName = "Client.xml";

		tinyxml2::XMLDocument doc;
		tinyxml2::XMLError error = doc.LoadFile(fileName.data());

		HB_ASSERT(error == tinyxml2::XML_SUCCESS, "Failed to read xml file: {0}", fileName);

		auto root = doc.RootElement();

		// 서버 포트 번호 읽기
		auto elem = root->FirstChildElement("Server")->FirstChildElement("Port");
		string port = elem->GetText();
		ServerPort = std::stoi(port);

		// 서버 IP 주소 읽기
		elem = elem->NextSiblingElement();
		ServerIP = elem->GetText();

		// 타일 크기
		elem = root->FirstChildElement("Values")->FirstChildElement("TileSide");
		string tileSide = elem->GetText();
		TileSide = stof(tileSide);

		// 플레이어 이동 속도
		elem = elem->NextSiblingElement();
		string playerSpeed = elem->GetText();
		PlayerSpeed = stof(playerSpeed);

		// 탱크 이동 속도
		elem = elem->NextSiblingElement();
		string tankSpeed = elem->GetText();
		TankSpeed = stof(tankSpeed);

		// 적 이동 속도
		elem = elem->NextSiblingElement();
		string enemySpeed = elem->GetText();
		EnemySpeed = stof(enemySpeed);

		// Cell 이동 속도
		elem = elem->NextSiblingElement();
		string cellSpeed = elem->GetText();
		CellSpeed = stof(cellSpeed);
	}

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

__declspec(selectany) uint16 Values::ServerPort = 0;
__declspec(selectany) string Values::ServerIP = "";
__declspec(selectany) float Values::TileSide = 0.0f;
__declspec(selectany) float Values::PlayerSpeed = 0.0f;
__declspec(selectany) float Values::TankSpeed = 0.0f;
__declspec(selectany) float Values::EnemySpeed = 0.0f;
__declspec(selectany) float Values::CellSpeed = 0.0f;
__declspec(selectany) uint32 Values::EntityID = 3;
__declspec(selectany) int Values::HostID = 0;
