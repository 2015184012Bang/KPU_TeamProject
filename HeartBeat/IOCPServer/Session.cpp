#include "pch.h"
#include "Session.h"

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

	// lpdwBytesReceived�� ���ڷ� NULL.
	// ���� ��Ŷ ũ�� + ���� �ּ� ũ�� + Ŭ�� �ּ� ũ�Ⱑ ������ ���⼭ ó�� ����.
	BOOL retVal = AcceptEx(listenSocket, mSocket, mAcceptBuf, 0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		NULL,
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
	mRecvContext.WsaBuf.len = RECV_BUFFER_SIZE;

	DWORD flags = 0;

	// 4��° ���ڸ� NULL�� ����
	// ������ �����Ͱ� �־ �ٷ� ���� �ʵ��� �Ѵ�.
	int retVal = WSARecv(mSocket,
		&mRecvContext.WsaBuf,
		1,
		NULL,
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
	// �� OVERLAPPED ����ü�� ����� ���� �����͸� �����Ѵ�.
	// �״��� SendQueue�� �ִ´�.
	// �ڿ� ������ SendCompleted()���� ����ȴ�.
	OVERLAPPEDEX* sendOver = new OVERLAPPEDEX;
	ZeroMemory(sendOver, sizeof(OVERLAPPEDEX));
	sendOver->Operation = IOOperation::SEND;
	sendOver->WsaBuf.buf = new char[dataSize];
	sendOver->WsaBuf.len = dataSize;
	CopyMemory(sendOver->WsaBuf.buf, msg, dataSize);

	WriteLockGuard guard(mSendLock);
	mSendQueue.push(sendOver);

	if (mSendQueue.size() == 1)
	{
		sendDataInQueue();
	}

	return true;
}

void Session::SendCompleted(const UINT32 dataSize)
{
	// TODO: �κ� Send�� ��� ������ ������ �����Ű��.

	LOG("SendCompleted: {0} bytes.", dataSize);

	// SendQueue���� ù ��° OVERLAPPED�� ���� �����Ѵ�.
	WriteLockGuard guard(mSendLock);
	OVERLAPPEDEX* sendOver = mSendQueue.front();
	delete[] sendOver->WsaBuf.buf;
	delete sendOver;
	mSendQueue.pop();

	// SendQueue�� ������� �ʴٸ� �ٽ� ������.
	if (!mSendQueue.empty())
	{
		sendDataInQueue();
	}
}

void Session::sendDataInQueue()
{
	OVERLAPPEDEX* sendOver = mSendQueue.front();

	int retVal = WSASend(mSocket,
		&sendOver->WsaBuf,
		1,
		NULL,
		0,
		reinterpret_cast<LPWSAOVERLAPPED>(sendOver),
		NULL);

	if (SOCKET_ERROR == retVal && (WSAGetLastError() != WSA_IO_PENDING))
	{
		LOG("WSASend() failed: {0}", WSAGetLastError());
	}
}
