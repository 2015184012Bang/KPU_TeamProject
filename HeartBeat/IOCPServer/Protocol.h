#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <DirectXTK12/SimpleMath.h>
using namespace DirectX::SimpleMath;

// 서버에서 사용
struct PACKET_INFO
{
	INT32 SessionIndex = -1;
	UINT8 PacketID = 0;
	UINT8 DataSize = 0;
	char* DataPtr = nullptr;
};

// 클라에서 사용
struct PACKET
{
	UINT8 PacketID = 0;
	UINT8 DataSize = 0;
	char* DataPtr = nullptr;
};

enum PACKET_ID : UINT8
{
	// 서버용
	SYS_USER_CONNECT = 11,
	SYS_USER_DISCONNECT = 12,
	SYS_END = 30,

	// 로그인
	REQUEST_LOGIN = 101,
	ANSWER_LOGIN = 102,
	NOTIFY_LOGIN = 103,
	
	// 업그레이드 씬
	REQUEST_ENTER_UPGRADE = 113,
	NOTIFY_ENTER_UPGRADE = 114,

	// 게임 씬
	REQUEST_ENTER_GAME = 116,
	NOTIFY_ENTER_GAME = 117,

	// 이동
	REQUEST_MOVE = 121,
	NOTIFY_MOVE,

	// 업그레이드
	REQUEST_UPGRADE = 131,
	NOTIFY_UPGRADE,

	// 공격
	REQUEST_ATTACK = 134,
	NOTIFY_ATTACK,
};

enum ERROR_CODE : UINT8
{
	SUCCESS = 0,
	ATTACK_NOT_YET = 1,
};



/************************************************************************/
/* 패킷 정의                                                             */
/************************************************************************/
#pragma pack(push, 1)
struct PACKET_HEADER
{
	UINT8 PacketSize;
	UINT8 PacketID;
};

constexpr UINT32 PACKET_HEADER_SIZE = sizeof(PACKET_HEADER);
constexpr UINT32 MAX_ID_LEN = 16;

struct REQUEST_LOGIN_PACKET : public PACKET_HEADER
{
	char ID[MAX_ID_LEN + 1];
};

struct ANSWER_LOGIN_PACKET : public PACKET_HEADER
{
	UINT8 Result;
	UINT8 ClientID;
};

struct NOTIFY_LOGIN_PACKET : public PACKET_HEADER
{
	UINT8 ClientID;
};

struct REQUEST_ENTER_UPGRADE_PACKET : public PACKET_HEADER
{

};

struct NOTIFY_ENTER_UPGRADE_PACKET : public PACKET_HEADER
{
	UINT8 Result;
};

struct REQUEST_ENTER_GAME_PACKET : public PACKET_HEADER
{

};

struct NOTIFY_ENTER_GAME_PACKET : public PACKET_HEADER
{
	UINT8 Result;
};

struct REQUEST_MOVE_PACKET : public PACKET_HEADER
{
	Vector3 Direction;
};

struct NOTIFY_MOVE_PACKET : public PACKET_HEADER
{
	UINT32 EntityID;
	Vector3 Position;
	Vector3 Direction;
};

struct REQUEST_UPGRADE_PACKET : public PACKET_HEADER
{
	UINT8 UpgradePreset;	// 0 : Attack Preset
							// 1 : Heal Preset
							// 2 : Support Preset
};

struct NOTIFY_UPGRADE_PACKET : public PACKET_HEADER
{
	UINT32 EntityID;
	UINT8 UpgradePreset;
};

struct REQUEST_ATTACK_PACKET : public PACKET_HEADER
{
	
};

struct NOTIFY_ATTACK_PACKET : public PACKET_HEADER
{
	UINT32 EntityID;
};

#pragma pack(pop)

