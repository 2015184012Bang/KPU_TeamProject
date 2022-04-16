#include "Session.h"
#include "../Log.h"

#include <chrono>

using namespace std::chrono;

Session::Session()
{
	ZeroMemory(&mAcceptContext, sizeof(mAcceptContext));
	ZeroMemory(&mRecvContext, sizeof(mRecvContext));
}

void Session::Init(const INT32 index, HANDLE iocpHandle)
{
	mIndex = index;
	mIOCPHandle = iocpHandle;
}

void Session::Close(bool bForce /*= false*/)
{
	linger opt = { 0, 0 };

	if (bForce)
	{
		opt.l_onoff = 1;
		setsockopt(mSocket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&opt), sizeof(opt));
	}

	shutdown(mSocket, SD_BOTH);
	closesocket(mSocket);

	mSocket = INVALID_SOCKET;
	mIsConnected = false;
	mLatestClosedTimeSec = duration_cast<seconds>(steady_clock::now().time_since_epoch()).count();
}

void Session::BindAccept(SOCKET listenSocket)
{
	LOG("BindAccept. Session index: {0}", mIndex);

	mLatestClosedTimeSec = UINT64_MAX;

	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (INVALID_SOCKET == mSocket)
	{
		LOG("Failed to create socket.");
		return;
	}

	ZeroMemory(&mAcceptContext, sizeof(mAcceptContext));
	mAcceptContext.Operation = IOOperation::ACCEPT;
	mAcceptContext.SessionIndex = mIndex;
	mAcceptContext.WsaBuf.buf = nullptr;
	mAcceptContext.WsaBuf.len = 0;

	DWORD numBytes = 0;
	BOOL retVal = AcceptEx(listenSocket, mSocket, mAcceptBuf, 0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&numBytes,
		(LPOVERLAPPED)&mAcceptContext);

	if (FALSE == retVal && (WSAGetLastError() != WSA_IO_PENDING))
	{
		LOG("AcceptEx Error: {0}", WSAGetLastError());
		return;
	}
}

bool Session::AcceptCompletion()
{
	LOG("AcceptCompletion. Session index: {0}", mIndex);

	HANDLE hIOCP = CreateIoCompletionPort(reinterpret_cast<HANDLE>(mSocket), 
		mIOCPHandle,
		(ULONG_PTR)this, 
		0);

	if (hIOCP != mIOCPHandle)
	{
		LOG("Failed to bind socket to IOCP");
		return false;
	}

	mIsConnected = true;

	return BindRecv();
}

bool Session::BindRecv()
{
	mRecvContext.Operation = IOOperation::RECV;
	mRecvContext.WsaBuf.buf = mRecvBuf;
	mRecvContext.WsaBuf.len = RECVBUF_SIZE;

	DWORD numBytes = 0;
	DWORD flags = 0;

	int retVal = WSARecv(mSocket,
		&mRecvContext.WsaBuf,
		1,
		&numBytes,
		&flags,
		(LPOVERLAPPED)&mRecvContext,
		NULL);

	if (SOCKET_ERROR == retVal && (WSAGetLastError() != WSA_IO_PENDING))
	{
		LOG("WSARecv() failed: {0}", WSAGetLastError());
		return false;
	}

	return true;
}

bool Session::SendMsg(const UINT32 dataSize, char* msg)
{
	// 새 OVERLAPPED 구조체를 만들고 보낼 데이터를 복사한다.
	// 그다음 SendQueue에 넣는다.
	// 자원 해제는 SendCompleted()에서 수행된다.
	OVERLAPPEDEX* sendOver = new OVERLAPPEDEX;
	ZeroMemory(sendOver, sizeof(OVERLAPPEDEX));
	sendOver->Operation = IOOperation::SEND;
	sendOver->WsaBuf.buf = new char[dataSize];
	sendOver->WsaBuf.len = dataSize;
	CopyMemory(sendOver->WsaBuf.buf, msg, dataSize);

	LockGuard guard(mSendLock);
	mSendQueue.push(sendOver);

	if (mSendQueue.size() == 1)
	{
		sendDataInQueue();
	}

	return true;
}

void Session::SendCompleted(const UINT32 dataSize)
{
	LOG("SendCompleted: {0} bytes.", dataSize);

	// SendQueue에서 첫 번째 OVERLAPPED를 꺼내 삭제한다.
	LockGuard guard(mSendLock);
	OVERLAPPEDEX* sendOver = mSendQueue.front();
	delete[] sendOver->WsaBuf.buf;
	delete sendOver;
	mSendQueue.pop();

	// SendQueue가 비어있지 않다면 다시 보낸다.
	if (!mSendQueue.empty())
	{
		sendDataInQueue();
	}
}

void Session::sendDataInQueue()
{
	OVERLAPPEDEX* sendOver = mSendQueue.front();

	DWORD numBytes = 0;

	int retVal = WSASend(mSocket,
		&sendOver->WsaBuf,
		1,
		&numBytes,
		0,
		reinterpret_cast<LPWSAOVERLAPPED>(sendOver),
		NULL);

	if (SOCKET_ERROR == retVal && (WSAGetLastError() != WSA_IO_PENDING))
	{
		LOG("WSASend() failed: {0}", WSAGetLastError());
	}
}
