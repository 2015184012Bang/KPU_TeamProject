#include "ClientPCH.h"
#include "Define.h"

#include "tinyxml2.h"

uint16 Values::ServerPort;
string Values::ServerIP;
float Values::TileSide;
float Values::PlayerSpeed;
float Values::TankSpeed;
float Values::EnemySpeed;
float Values::CellSpeed;
uint32 Values::EntityID = 3;
int Values::HostID = 0;

void Values::Init()
{
	string fileName = "Client.xml";

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(fileName.data());

	HB_ASSERT(error == tinyxml2::XML_SUCCESS, "Failed to read xml file: {0}", fileName);

	auto root = doc.RootElement();

	// ���� ��Ʈ ��ȣ �б�
	auto elem = root->FirstChildElement("Server")->FirstChildElement("Port");
	string port = elem->GetText();
	ServerPort = std::stoi(port);

	// ���� IP �ּ� �б�
	elem = elem->NextSiblingElement();
	ServerIP = elem->GetText();

	// Ÿ�� ũ��
	elem = root->FirstChildElement("Values")->FirstChildElement("TileSide");
	string tileSide = elem->GetText();
	TileSide = stof(tileSide);

	// �÷��̾� �̵� �ӵ�
	elem = elem->NextSiblingElement();
	string playerSpeed = elem->GetText();
	PlayerSpeed = stof(playerSpeed);

	// ��ũ �̵� �ӵ�
	elem = elem->NextSiblingElement();
	string tankSpeed = elem->GetText();
	TankSpeed = stof(tankSpeed);

	// �� �̵� �ӵ�
	elem = elem->NextSiblingElement();
	string enemySpeed = elem->GetText();
	EnemySpeed = stof(enemySpeed);

	// Cell �̵� �ӵ�
	elem = elem->NextSiblingElement();
	string cellSpeed = elem->GetText();
	CellSpeed = stof(cellSpeed);
}
