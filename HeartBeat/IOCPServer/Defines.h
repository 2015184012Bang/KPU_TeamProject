#pragma once

#include <WinSock2.h>

enum class IOOperation
{
	NONE,
	ACCEPT,
	RECV,
	SEND
};

struct OVERLAPPEDEX
{
	WSAOVERLAPPED Over = {};
	WSABUF WsaBuf = {};
	IOOperation Operation = IOOperation::NONE;
	INT32 SessionIndex = -1;
};

enum class UpgradePreset : UINT8
{
	ATTACK = 0,
	HEAL,
	SUPPORT,
};

enum class BossSkill : INT8
{
	SKILL_1,
	SKILL_2,
	SKILL_SPECIAL,
	NONE,
};