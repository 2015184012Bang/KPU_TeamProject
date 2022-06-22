#include "pch.h"
#include "Values.h"

#include "tinyxml2.h"

UINT16 Values::ServerPort;
float Values::TileSide;
float Values::PlayerSpeed;
float Values::BaseAttackCooldown;
float Values::SkillCooldown;
float Values::BaseAttackRange;
float Values::TankSpeed;
float Values::EnemySpeed;
float Values::CellSpeed;
UINT8 Values::TankHealth;
UINT8 Values::EnemyHealth;
UINT8 Values::PlayerHealth;
UINT8 Values::CellHealth;
UINT32 Values::EntityID = 3;
UINT8 Values::MaxRoomNum = 3;

void Values::Init()
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

	// 스킬 재사용 대기 시간
	elem = elem->NextSiblingElement();
	string skillCooldown = elem->GetText();
	Values::SkillCooldown = stof(skillCooldown);

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

	// Cell 최대 체력
	elem = elem->NextSiblingElement();
	string cellHealth = elem->GetText();
	Values::CellHealth = stoi(cellHealth);
}

