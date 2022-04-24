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
	ANSWER_NOTIFY_LOGIN = 103,
	
	// 게임 시작
	REQUEST_GAME_START = 113,
	ANSWER_GAME_START = 114,

	// 이동
	REQUEST_MOVE = 121,
	ANSWER_NOTIFY_MOVE = 122,
};

enum ERROR_CODE : UINT8
{
	SUCCESS = 0,

	START_GAME = 10,
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

struct ANSWER_NOTIFY_LOGIN_PACKET : public PACKET_HEADER
{
	UINT8 ClientID;
};

struct REQUEST_GAME_START_PACKET : public PACKET_HEADER
{

};

struct ANSWER_GAME_START_PACKET : public PACKET_HEADER
{
	UINT8 Result;
};

struct REQUEST_MOVE_PACKET : public PACKET_HEADER
{
	Vector3 Direction;
};

struct ANSWER_NOTIFY_MOVE_PACKET : public PACKET_HEADER
{
	UINT32 EntityID;
	Vector3 Direction;
};

#pragma pack(pop)

