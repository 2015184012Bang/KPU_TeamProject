#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "tinyxml2.h"

class Values
{
public:
	static void Init()
	{
		string fileName = "settings.xml";

		tinyxml2::XMLDocument doc;
		tinyxml2::XMLError error = doc.LoadFile(fileName.data());
		ASSERT(error == tinyxml2::XML_SUCCESS, "Failed to read xml file: {0}", fileName);

		auto root = doc.RootElement();

		// 타일 한 변의 길이
		auto elem = root->FirstChildElement("Values")->FirstChildElement("TileSide");
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

		// 탱크 최대 체력
		elem = elem->NextSiblingElement();
		string tankHealth = elem->GetText();
		Values::TankHealth = stoi(tankHealth);
	}

	static float TileSide;
	static float PlayerSpeed;
	static float BaseAttackCooldown;
	static float BaseAttackRange;
	static float TankSpeed;
	static UINT8 TankHealth;
	static UINT32 EntityID;
};

__declspec(selectany) float Values::TileSide = 0.0f;
__declspec(selectany) float Values::PlayerSpeed = 0.0f;
__declspec(selectany) float Values::BaseAttackCooldown = 0.0f;
__declspec(selectany) float Values::BaseAttackRange = 0.0f;
__declspec(selectany) float Values::TankSpeed = 0.0f;
__declspec(selectany) UINT32 Values::EntityID = 3;
__declspec(selectany) UINT8 Values::TankHealth = 0;
