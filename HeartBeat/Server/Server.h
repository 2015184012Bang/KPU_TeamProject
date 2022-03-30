#pragma once

#include "Game.h"

constexpr int NUM_MAX_PLAYER = 1;

class Server : public Game
{
	struct Session
	{
		Session(bool connect, TCPSocketPtr socket, const SocketAddress& addr)
			: bConnect(connect)
			, ClientSocket(socket)
			, ClientAddr(addr) {}

		bool bConnect;
		TCPSocketPtr ClientSocket;
		SocketAddress ClientAddr;
	};

public:
	Server();

	virtual bool Init() override;
	virtual void Shutdown() override;
	virtual void Run() override;

private:
	void accpetClients();

	void processPacket(MemoryStream* outPacket, TCPSocketPtr& clientSocket);
	void processLoginRequest(MemoryStream* outPacket, TCPSocketPtr& clientSocket);

private:
	TCPSocketPtr mListenSocket;

	vector<Session> mSessions;

	bool mbGameStart;
};