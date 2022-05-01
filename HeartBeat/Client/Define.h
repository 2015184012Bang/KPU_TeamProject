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
	}

	static uint16 ServerPort;
	static string ServerIP;
	static float TileSide;
	static float PlayerSpeed;
	static float TankSpeed;
	static uint32 EntityID;
	static int HostID;
};

__declspec(selectany) uint16 Values::ServerPort = 0;
__declspec(selectany) string Values::ServerIP = "";
__declspec(selectany) float Values::TileSide = 0.0f;
__declspec(selectany) float Values::PlayerSpeed = 0.0f;
__declspec(selectany) float Values::TankSpeed = 0.0f;
__declspec(selectany) uint32 Values::EntityID = 3;
__declspec(selectany) int Values::HostID = 2;
