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

	// 로그인
	REQUEST_LOGIN,
	ANSWER_LOGIN,

	// 룸 씬
	REQUEST_ROOM,
	NOTIFY_ROOM,
	REQUEST_ENTER_ROOM,
	ANSWER_ENTER_ROOM,
	NOTIFY_ENTER_ROOM,

	REQUEST_LEAVE_ROOM,
	NOTIFY_LEAVE_ROOM,

	// 업그레이드 씬
	REQUEST_ENTER_UPGRADE,
	NOTIFY_ENTER_UPGRADE,

	// 게임 씬
	REQUEST_ENTER_GAME,
	NOTIFY_ENTER_GAME ,

	// 이동
	REQUEST_MOVE,
	NOTIFY_MOVE,

	// 업그레이드
	REQUEST_UPGRADE,
	NOTIFY_UPGRADE,

	// 공격
	REQUEST_ATTACK,
	NOTIFY_ATTACK,
	NOTIFY_ENEMY_ATTACK,

	REQUEST_SKILL,
	NOTIFY_SKILL,

	// 생성/삭제
	NOTIFY_CREATE_ENTITY,
	NOTIFY_DELETE_ENTITY,

	NOTIFY_STATE_CHANGE,

	// 게임오버
	NOTIFY_GAME_OVER,

	// 이벤트
	NOTIFY_EVENT_OCCUR,
};

enum RESULT_CODE : UINT8
{
	SUCCESS = 0,

	// 룸
	ROOM_ENTER_DENY,
	ROOM_ENTER_SUCCESS,

	// 공격 에러 코드
	ATTACK_SUCCESS,
	ATTACK_MISS,

	// 게임 오버 에러 코드
	STAGE_CLEAR,
	STAGE_FAIL,
};

enum class EntityType : UINT8
{
	FAT,
	TANK,
	CART,
	VIRUS,
	DOG,
	PLAYER,
	RED_CELL,
	VITAMIN,
	CAFFEINE,
	END
};

enum class EventType : UINT8
{
	DOOR_DOWN,
	PLAYER_DEAD,
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
};

struct REQUEST_ROOM_PACKET : public PACKET_HEADER
{

};

constexpr UINT8 MAX_ROOM_NUM = 32;

struct NOTIFY_ROOM_PACKET : public PACKET_HEADER
{
	char Room[MAX_ROOM_NUM]; // 0이면 입장 가능한 방,
							 // 1이면 입장 불가능 방
};

struct REQUEST_ENTER_ROOM_PACKET : public PACKET_HEADER 
{
	UINT8 RoomNumber;
};

struct ANSWER_ENTER_ROOM_PACKET : public PACKET_HEADER
{
	UINT8 Result;
	UINT8 ClientID;
};

struct NOTIFY_ENTER_ROOM_PACKET : public PACKET_HEADER
{
	UINT8 ClientID;
	char UserName[MAX_ID_LEN + 1];
};

struct REQUEST_LEAVE_ROOM_PACKET : public PACKET_HEADER
{

};

struct NOTIFY_LEAVE_ROOM_PACKET : public PACKET_HEADER 
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
	UINT8 Result;
};

struct NOTIFY_ENEMY_ATTACK_PACKET : public PACKET_HEADER
{
	UINT32 HitterID;
	UINT32 VictimID;
};

struct REQUEST_SKILL_PACKET : public PACKET_HEADER
{

};

struct NOTIFY_SKILL_PACKET : public PACKET_HEADER
{
	UINT32 EntityID;
	UINT8 Preset;
};

struct NOTIFY_CREATE_ENTITY_PACKET : public PACKET_HEADER
{
	UINT32 EntityID;
	UINT8 EntityType;
	Vector3 Position;
};

struct NOTIFY_DELETE_ENTITY_PACKET : public PACKET_HEADER
{
	UINT32 EntityID;
	UINT8 EntityType;
};

struct NOTIFY_STATE_CHANGE_PACKET : public PACKET_HEADER
{
	INT32 O2;
	INT32 CO2;
	INT8 TankHealth;
	INT8 P0Health;
	INT8 P1Health;
	INT8 P2Health;
};

struct NOTIFY_GAME_OVER_PACKET : public PACKET_HEADER
{
	INT32 O2;
	INT32 CO2;
	UINT64 PlayTimeSec;
};

struct NOTIFY_EVENT_OCCUR_PACKET : public PACKET_HEADER
{
	UINT8 EventType;
	INT32 AdditionalData;
};

#pragma pack(pop)

