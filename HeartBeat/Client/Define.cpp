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
