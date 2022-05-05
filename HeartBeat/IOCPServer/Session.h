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
	// ���� �ε���
	INT32 mIndex = -1;

	// IOCP �ڵ�
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;

	// ���� ����
	bool mIsConnected = false;

	// Accpet�� �ɾ���� �� = UINT64_MAX
	// ���� ���� �� = ���� ���� ����
	UINT64 mLatestClosedTimeSec = 0;

	// Ŭ���̾�Ʈ ����
	SOCKET mSocket = INVALID_SOCKET;

	// AcceptEx() ȣ�⿡ ���
	OVERLAPPEDEX mAcceptContext = {};
	char mAcceptBuf[64] = { 0, };

	// WSARecv() ȣ�⿡ ���
	OVERLAPPEDEX mRecvContext = {};
	char mRecvBuf[RECV_BUFFER_SIZE] = { 0, };
};