#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <DirectXTK12/SimpleMath.h>
using namespace DirectX::SimpleMath;

// �������� ���
struct PACKET_INFO
{
	INT32 SessionIndex = -1;
	UINT8 PacketID = 0;
	UINT8 DataSize = 0;
	char* DataPtr = nullptr;
};

// Ŭ�󿡼� ���
struct PACKET
{
	UINT8 PacketID = 0;
	UINT8 DataSize = 0;
	char* DataPtr = nullptr;
};

enum PACKET_ID : UINT8
{
	// ������
	SYS_USER_CONNECT = 11,
	SYS_USER_DISCONNECT = 12,

	// �α���
	REQUEST_LOGIN,
	ANSWER_LOGIN,
	NOTIFY_LOGIN,
	
	// ���׷��̵� ��
	REQUEST_ENTER_UPGRADE,
	NOTIFY_ENTER_UPGRADE,

	// ���� ��
	REQUEST_ENTER_GAME,
	NOTIFY_ENTER_GAME ,

	// �̵�
	REQUEST_MOVE,
	NOTIFY_MOVE,

	// ���׷��̵�
	REQUEST_UPGRADE,
	NOTIFY_UPGRADE,

	// ����
	REQUEST_ATTACK,
	NOTIFY_ATTACK,

	// ����/����
	NOTIFY_CREATE_ENTITY,
	NOTIFY_DELETE_ENTITY,

	// ���ӿ���
	NOTIFY_GAME_OVER,
};

enum RESULT_CODE : UINT8
{
	SUCCESS = 0,

	// ���� ���� �ڵ�
	ATTACK_SUCCESS,
	ATTACK_MISS,

	// ���� ���� ���� �ڵ�
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
	END
};

/************************************************************************/
/* ��Ŷ ����                                                             */
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
	UINT8 Result;
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

struct NOTIFY_GAME_OVER_PACKET : public PACKET_HEADER
{
	UINT8 Result;
};

#pragma pack(pop)

