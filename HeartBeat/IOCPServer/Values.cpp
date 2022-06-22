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

	// ��ų ���� ��� �ð�
	elem = elem->NextSiblingElement();
	string skillCooldown = elem->GetText();
	Values::SkillCooldown = stof(skillCooldown);

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

	// Cell �ִ� ü��
	elem = elem->NextSiblingElement();
	string cellHealth = elem->GetText();
	Values::CellHealth = stoi(cellHealth);
}

