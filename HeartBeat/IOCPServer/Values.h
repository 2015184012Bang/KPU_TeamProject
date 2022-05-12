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

		// Ÿ�� �� ���� ����
		elem = root->FirstChildElement("Values")->FirstChildElement("TileSide");
		string tileSide = elem->GetText();
		Values::TileSide = stof(tileSide);

		// �÷��̾� �̵� �ӵ�
		elem = elem->NextSiblingElement();
		string playerSpeed = elem->GetText();
		Values::PlayerSpeed = stof(playerSpeed);

		// �⺻ ���� ���� ��� �ð�
		elem = elem->NextSiblingElement();
		string baCooldown = elem->GetText();
		Values::BaseAttackCooldown = stof(baCooldown);

		// �⺻ ���� ���� ��Ÿ�
		elem = elem->NextSiblingElement();
		string baRange = elem->GetText();
		Values::BaseAttackRange = stof(baRange);

		// ��ũ �̵� �ӵ�
		elem = elem->NextSiblingElement();
		string tankSpeed = elem->GetText();
		Values::TankSpeed = stof(tankSpeed);

		// �� �̵� �ӵ�
		elem = elem->NextSiblingElement();
		string enemySpeed = elem->GetText();
		Values::EnemySpeed = stof(enemySpeed);

		// Cell �̵� �ӵ�
		elem = elem->NextSiblingElement();
		string cellSpeed = elem->GetText();
		Values::CellSpeed = stof(cellSpeed);

		// ��ũ �ִ� ü��
		elem = elem->NextSiblingElement();
		string tankHealth = elem->GetText();
		Values::TankHealth = stoi(tankHealth);

		// �� �ִ� ü��
		elem = elem->NextSiblingElement();
		string enemyHealth = elem->GetText();
		Values::EnemyHealth = stoi(enemyHealth);

		// �÷��̾� �ִ� ü��
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
