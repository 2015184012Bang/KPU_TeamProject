#pragma once

#include "../Types.h"
#include "NetDefine.h"
#include "Session.h"

#include <vector>
#include <thread>

using std::vector;
using std::thread;

constexpr INT32 MAX_WORKER_THREADS = 4;
constexpr UINT64 REUSE_WAIT_TIMEOUT = 3;

class IOCPServer
{
public:
	virtual ~IOCPServer();

	void Init();

	void BindAndListen(const UINT16 bindPort);

	bool SendMsg(const INT32 sessionIndex, const UINT32 dataSize, char* msg);

	virtual void End();

protected:
	void StartServer(const UINT32 maxSessionCount);

	virtual void OnConnect(const INT32 sessionIndex) {}
	
	virtual void OnClose(const INT32 sessionIndex) {}
	
	virtual void OnRecv(const INT32 sessionIndex, const UINT32 dataSize, char* msg) {}

private:
	void createSessions(const UINT32 maxSession);

	void createWorkerThread();

	void createAccepterThread();

	void workerThread();

	void accepterThread();

	Session* getEmptySession();

	Session* getSession(const INT32 sessionIndex);

	void closeSession(Session* session, bool bForce = false);

private:
	vector<Session*> mSessions;

	// 현재 연결된 세션 수
	UINT32 mSessionCount = 0;

	vector<thread> mWorkerThreads;

	thread mAccepterThread;

	SOCKET mListenSocket = INVALID_SOCKET;

	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;

	bool mShouldWorkerRun = true;

	bool mShouldAccepterRun = true;

	bool mIsEnd = false;
};

