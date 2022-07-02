#include "pch.h"
#include "Values.h"

#include "tinyxml2.h"

UINT16 Values::ServerPort;
float Values::TileSide;
UINT8 Values::TileMaxHealth;
float Values::PlayerSpeed;
UINT8 Values::PlayerHealth;
UINT8 Values::PlayerLife;
float Values::BaseAttackCooldown;
UINT8 Values::BaseAttackPower;
float Values::BaseAttackRange;
float Values::SkillSlashCooldown;
float Values::SkillHealCooldown;
float Values::SkillBuffCooldown;
float Values::SkillBuffDuration;
float Values::TankSpeed;
UINT8 Values::TankHealth;
float Values::CellSpeed;
UINT8 Values::CellPower;
UINT8 Values::CellHealth;
float Values::EnemySpeed;
UINT8 Values::VirusHealth;
UINT8 Values::VirusPower;
UINT8 Values::DogHealth;
UINT8 Values::DogPower;
UINT8 Values::BossHealth;
UINT8 Values::BossPower;
UINT8 Values::ItemDrop;
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
	Values::TileSide = stof(elem->GetText());

	// Ÿ�� �ִ� ü��
	elem = root->FirstChildElement("Values")->FirstChildElement("TileMaxHealth");
	Values::TileMaxHealth = stoi(elem->GetText());

	// �÷��̾� �̵��ӵ�
	elem = root->FirstChildElement("Values")->FirstChildElement("PlayerSpeed");
	Values::PlayerSpeed = stof(elem->GetText());

	// �÷��̾� ü��
	elem = root->FirstChildElement("Values")->FirstChildElement("PlayerHealth");
	Values::PlayerHealth = stoi(elem->GetText());

	// �÷��̾� ����
	elem = root->FirstChildElement("Values")->FirstChildElement("PlayerLife");
	Values::PlayerLife = stoi(elem->GetText());

	// �⺻ ���� ��ٿ�
	elem = root->FirstChildElement("Values")->FirstChildElement("BACooldown");
	Values::BaseAttackCooldown = stof(elem->GetText());

	// �⺻ ���ݷ�
	elem = root->FirstChildElement("Values")->FirstChildElement("BAPower");
	Values::BaseAttackPower = stoi(elem->GetText());

	// �⺻ ���� ��Ÿ�
	elem = root->FirstChildElement("Values")->FirstChildElement("BARange");
	Values::BaseAttackRange = stof(elem->GetText());

	// ������(����) ��ų ��ٿ�
	elem = root->FirstChildElement("Values")->FirstChildElement("Skill_Slash_CD");
	Values::SkillSlashCooldown = stof(elem->GetText());

	// ��(����) ��ų ��ٿ�
	elem = root->FirstChildElement("Values")->FirstChildElement("Skill_Heal_CD");
	Values::SkillHealCooldown = stof(elem->GetText());

	// ����(����) ��ų ��ٿ�
	elem = root->FirstChildElement("Values")->FirstChildElement("Skill_Buff_CD");
	Values::SkillBuffCooldown = stof(elem->GetText());

	// ���� ��ų ���ӽð�
	elem = root->FirstChildElement("Values")->FirstChildElement("Skill_Buff_DR");
	Values::SkillBuffDuration = stof(elem->GetText());

	// ��ũ �̵��ӵ�
	elem = root->FirstChildElement("Values")->FirstChildElement("TankSpeed");
	Values::TankSpeed = stof(elem->GetText());

	// ��ũ ü��
	elem = root->FirstChildElement("Values")->FirstChildElement("TankHealth");
	Values::TankHealth = stoi(elem->GetText());

	// ���� �̵��ӵ�
	elem = root->FirstChildElement("Values")->FirstChildElement("CellSpeed");
	Values::CellSpeed = stof(elem->GetText());

	// ���� ���ݷ�
	elem = root->FirstChildElement("Values")->FirstChildElement("CellPower");
	Values::CellPower = stoi(elem->GetText());

	// ���� ü��
	elem = root->FirstChildElement("Values")->FirstChildElement("CellHealth");
	Values::CellHealth = stoi(elem->GetText());

	// �� �̵��ӵ�
	elem = root->FirstChildElement("Values")->FirstChildElement("EnemySpeed");
	Values::EnemySpeed = stof(elem->GetText());

	// ���̷��� ü��
	elem = root->FirstChildElement("Values")->FirstChildElement("VirusHealth");
	Values::VirusHealth = stoi(elem->GetText());

	// ���̷��� ���ݷ�
	elem = root->FirstChildElement("Values")->FirstChildElement("VirusPower");
	Values::VirusPower = stoi(elem->GetText());

	// �� ü��
	elem = root->FirstChildElement("Values")->FirstChildElement("DogHealth");
	Values::DogHealth = stoi(elem->GetText());

	// �� ���ݷ�
	elem = root->FirstChildElement("Values")->FirstChildElement("DogPower");
	Values::DogPower = stoi(elem->GetText());

	// ���� ü��
	elem = root->FirstChildElement("Values")->FirstChildElement("BossHealth");
	Values::BossHealth = stoi(elem->GetText());

	// ���� ���ݷ�
	elem = root->FirstChildElement("Values")->FirstChildElement("BossPower");
	Values::BossPower = stoi(elem->GetText());
	
	// ������ �����
	elem = root->FirstChildElement("Values")->FirstChildElement("ItemDrop");
	Values::ItemDrop = stoi(elem->GetText());
}
