#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

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
	SYS_END = 30,

	// 
	REQUEST_LOGIN = 101,
	ANSWER_LOGIN = 102,
};

enum ERROR_CODE : UINT8
{
	SUCCESS = 0,
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
#pragma pack(pop)
