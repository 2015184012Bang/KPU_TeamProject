#pragma once

#include "Defines.h"


#include <queue>

using std::queue;

constexpr UINT32 RECV_BUFFER_SIZE = 256;

class Session
{
public:
	Session();
	~Session() = default;

	void Init(const INT32 index, HANDLE iocpHandle);

	void Close(bool bForce = false);

	void BindAccept(SOCKET listenSocket);

	bool AcceptCompletion();

	bool BindRecv();

	bool SendMsg(const UINT32 dataSize, char* msg);

public:
	INT32 GetIndex() { return mIndex; }
	bool IsConnected() { return mIsConnected; }
	UINT64 GetLastestClosedTimeSec() { return mLatestClosedTimeSec; }
	SOCKET GetSocket() { return mSocket; }
	char* GetRecvBuffer() { return mRecvBuf; }

private:
	// 세션 인덱스
	INT32 mIndex = -1;

	// IOCP 핸들
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;

	// 연결 여부
	bool mIsConnected = false;

	// Accpet를 걸어놨을 시 = UINT64_MAX
	// 연결 종료 시 = 연결 종료 시점
	UINT64 mLatestClosedTimeSec = 0;

	// 클라이언트 소켓
	SOCKET mSocket = INVALID_SOCKET;

	// AcceptEx() 호출에 사용
	OVERLAPPEDEX mAcceptContext = {};
	char mAcceptBuf[64] = { 0, };

	// WSARecv() 호출에 사용
	OVERLAPPEDEX mRecvContext = {};
	char mRecvBuf[RECV_BUFFER_SIZE] = { 0, };
};