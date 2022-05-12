#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "tinyxml2.h"

class Values
{
public:
	static void Init()
	{
		string fileName = "Server.xml";

		tinyxml2::XMLDocument doc;
		tinyxml2::XMLError error = doc.LoadFile(fileName.data());
		ASSERT(error == tinyxml2::XML_SUCCESS, "Failed to read xml file: {0}", fileName);

		auto root = doc.RootElement();

		auto elem = root->FirstChildElement("Server")->FirstChildElement("Port");
		string port = elem->GetText();
		Values::ServerPort = stoi(port);

		// 타일 한 변의 길이
		elem = root->FirstChildElement("Values")->FirstChildElement("TileSide");
		string tileSide = elem->GetText();
		Values::TileSide = stof(tileSide);

		// 플레이어 이동 속도
		elem = elem->NextSiblingElement();
		string playerSpeed = elem->GetText();
		Values::PlayerSpeed = stof(playerSpeed);

		// 기본 공격 재사용 대기 시간
		elem = elem->NextSiblingElement();
		string baCooldown = elem->GetText();
		Values::BaseAttackCooldown = stof(baCooldown);

		// 기본 공격 전방 사거리
		elem = elem->NextSiblingElement();
		string baRange = elem->GetText();
		Values::BaseAttackRange = stof(baRange);

		// 탱크 이동 속도
		elem = elem->NextSiblingElement();
		string tankSpeed = elem->GetText();
		Values::TankSpeed = stof(tankSpeed);

		// 적 이동 속도
		elem = elem->NextSiblingElement();
		string enemySpeed = elem->GetText();
		Values::EnemySpeed = stof(enemySpeed);

		// Cell 이동 속도
		elem = elem->NextSiblingElement();
		string cellSpeed = elem->GetText();
		Values::CellSpeed = stof(cellSpeed);

		// 탱크 최대 체력
		elem = elem->NextSiblingElement();
		string tankHealth = elem->GetText();
		Values::TankHealth = stoi(tankHealth);

		// 적 최대 체력
		elem = elem->NextSiblingElement();
		string enemyHealth = elem->GetText();
		Values::EnemyHealth = stoi(enemyHealth);

		// 플레이어 최대 체력
		elem = elem->NextSiblingElement();
		string playerHealth = elem->GetText();
		Values::PlayerHealth = stoi(playerHealth);
	}

	static UINT16 ServerPort;
	static float TileSide;
	static float PlayerSpeed;
	static float BaseAttackCooldown;
	static float BaseAttackRange;
	static float TankSpeed;
	static float EnemySpeed;
	static float CellSpeed;
	static UINT8 TankHealth;
	static UINT8 EnemyHealth;
	static UINT8 PlayerHealth;
	static UINT32 EntityID;
	static UINT8 MaxRoomNum;
};

__declspec(selectany) UINT16 Values::ServerPort = 0;
__declspec(selectany) float Values::TileSide = 0.0f;
__declspec(selectany) float Values::PlayerSpeed = 0.0f;
__declspec(selectany) float Values::BaseAttackCooldown = 0.0f;
__declspec(selectany) float Values::BaseAttackRange = 0.0f;
__declspec(selectany) float Values::TankSpeed = 0.0f;
__declspec(selectany) float Values::EnemySpeed = 0.0f;
__declspec(selectany) float Values::CellSpeed = 0.0f;
__declspec(selectany) UINT8 Values::TankHealth = 0;
__declspec(selectany) UINT8 Values::EnemyHealth = 0;
__declspec(selectany) UINT8 Values::PlayerHealth = 0;
__declspec(selectany) UINT32 Values::EntityID = 3;
__declspec(selectany) UINT8 Values::MaxRoomNum = 6;
