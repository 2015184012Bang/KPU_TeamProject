#include "pch.h"
#include "IOCPServer.h"

#include "tinyxml2.h"

IOCPServer::~IOCPServer()
{
	if (!mIsEnd)
	{
		End();
	}

	WSACleanup();
}

void IOCPServer::Init()
{
	WSADATA wsa;

	int retVal = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (0 != retVal)
	{
		LOG("WSAStartup() Failed: {0}", WSAGetLastError());
		return;
	}

	mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == mListenSocket)
	{
		LOG("Failed to create listen socket: {0}", WSAGetLastError());
		return;
	}

	BOOL opt = TRUE;
	setsockopt(mListenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), sizeof(opt));

	LOG("Winsock init success...");
}

void IOCPServer::BindAndListen()
{
	UINT16 bindPort = getPortNumber("settings.xml");

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(bindPort);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int retVal = ::bind(mListenSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr));
	if (SOCKET_ERROR == retVal)
	{
		LOG("Failed to bind listen socket: {0}", WSAGetLastError());
		return;
	}

	retVal = listen(mListenSocket, SOMAXCONN);
	if (SOCKET_ERROR == retVal)
	{
		LOG("Failed to listen: {0}", WSAGetLastError());
		return;
	}

	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (NULL == mIOCPHandle)
	{
		LOG("Failed to create iocp handle: {0}", GetLastError());
		return;
	}

	HANDLE hIOCP = CreateIoCompletionPort(reinterpret_cast<HANDLE>(mListenSocket),
		mIOCPHandle, 0, 0);
	if (hIOCP != mIOCPHandle)
	{
		LOG("Failed to register listen socket to iocp: {0}", GetLastError());
		return;
	}

	LOG("Bind and Listen success...");
}

void IOCPServer::StartServer(const UINT32 maxSessionCount)
{
	createSessions(maxSessionCount);
	createWorkerThread();
	createAccepterThread();

	LOG("Starting server...");
}

bool IOCPServer::SendMsg(const INT32 sessionIndex, const UINT32 dataSize, char* msg)
{
	auto session = mSessions[sessionIndex];
	return session->SendMsg(dataSize, msg);
}

void IOCPServer::End()
{
	mIsEnd = true;

	mShouldWorkerRun = false;
	CloseHandle(mIOCPHandle);

	for (auto& th : mWorkerThreads)
	{
		if (th.joinable())
		{
			th.join();
		}
	}

	mShouldAccepterRun = false;
	closesocket(mListenSocket);

	if (mAccepterThread.joinable())
	{
		mAccepterThread.join();
	}
}

void IOCPServer::createSessions(const UINT32 maxSession)
{
	mSessions.reserve(maxSession);
	for (UINT32 i = 0; i < maxSession; ++i)
	{
		auto session = new Session;
		session->Init(i, mIOCPHandle);
		mSessions.push_back(session);
	}
}

void IOCPServer::createWorkerThread()
{
	mWorkerThreads.reserve(MAX_WORKER_THREADS);
	for (INT32 i = 0; i < MAX_WORKER_THREADS; ++i)
	{
		mWorkerThreads.emplace_back(thread{ [this]() { workerThread(); } });
	}
}

void IOCPServer::createAccepterThread()
{
	mAccepterThread = thread{ [this]() { accepterThread(); } };
}

void IOCPServer::workerThread()
{
	Session* session = nullptr;
	LPOVERLAPPED lpOverlapped = nullptr;
	DWORD numBytes = 0;
	BOOL bSuccess = TRUE;

	while (mShouldWorkerRun)
	{
		bSuccess = GetQueuedCompletionStatus(mIOCPHandle, &numBytes,
			(PULONG_PTR)&session, &lpOverlapped, INFINITE);

		if (TRUE == bSuccess && 0 == numBytes && nullptr == lpOverlapped)
		{
			mShouldWorkerRun = false;
			continue;
		}

		OVERLAPPEDEX* overEx = reinterpret_cast<OVERLAPPEDEX*>(lpOverlapped);

		if (FALSE == bSuccess || (0 == numBytes && IOOperation::ACCEPT != overEx->Operation))
		{
			closeSession(session);
			continue;
		}

		switch (overEx->Operation)
		{
		case IOOperation::ACCEPT:
		{
			session = getSession(overEx->SessionIndex);
			if (session->AcceptCompletion())
			{
				++mSessionCount;
				OnConnect(session->GetIndex());
			}
			else
			{
				closeSession(session, true);
			}
			break;
		}
		case IOOperation::RECV:
		{
			OnRecv(session->GetIndex(), numBytes, session->GetRecvBuffer());
			session->BindRecv();
			break;
		}
		case IOOperation::SEND:
		{
			delete[] overEx->WsaBuf.buf;
			delete overEx;
			break;
		}
		default:
			LOG("Unknown io operation type!");
			break;
		}
	}
}

void IOCPServer::accepterThread()
{
	while (mShouldAccepterRun)
	{
		auto curTimeSec = duration_cast<seconds>(steady_clock::now().time_since_epoch()).count();

		for (auto session : mSessions)
		{
			if (session->IsConnected()) continue; // 연결되어있다면 X
			if ((UINT64)curTimeSec < session->GetLastestClosedTimeSec()) continue; // Accept 걸어놨다면 X

			auto timeDiff = curTimeSec - session->GetLastestClosedTimeSec(); 
			if (timeDiff <= REUSE_WAIT_TIMEOUT) continue; // 아직 일정 시간이 지나지 않았다면 X

			session->BindAccept(mListenSocket);
		}

		std::this_thread::sleep_for(32ms);
	}
}

Session* IOCPServer::getEmptySession()
{
	for (auto session : mSessions)
	{
		if (session->IsConnected() == false)
		{
			return session;
		}
	}

	LOG("There is no empty session!");
	return nullptr;
}

Session* IOCPServer::getSession(const INT32 sessionIndex)
{
	ASSERT(sessionIndex >= 0 && sessionIndex < mSessions.size(), "heelo, world!");
	return mSessions[sessionIndex];
}

void IOCPServer::closeSession(Session* session, bool bForce /*= false*/)
{
	if (session->IsConnected() == false)
	{
		return;
	}

	session->Close(bForce);
	OnClose(session->GetIndex());
}

UINT16 IOCPServer::getPortNumber(string_view fileName)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(fileName.data());

	ASSERT(error == tinyxml2::XML_SUCCESS, "Failed to read xml file: {0}", fileName.data());

	auto root = doc.RootElement();

	auto elem = root->FirstChildElement("Server")->FirstChildElement("Port");
	string port = elem->GetText();
	return static_cast<UINT16>(std::stoi(port));
}
