#pragma once

#include "HeartBeat/Game.h"

constexpr int NUM_MAX_PLAYER = 1;

class Server : public Game
{
	struct Session
	{
		Session(bool connect, TCPSocketPtr socket, const SocketAddress& addr, int id)
			: bConnect(connect)
			, ClientSocket(socket)
			, ClientAddr(addr)
			, ClientID(id)
		{}

		bool bConnect;
		TCPSocketPtr ClientSocket;
		SocketAddress ClientAddr;
		int ClientID;
	};

public:
	Server();

	virtual bool Init() override;
	virtual void Shutdown() override;
	virtual void Run() override;

private:
	void acceptClients();

	void processPacket(MemoryStream* outPacket, const Session& session);
	void processLoginRequest(MemoryStream* outPacket, const Session& session);
	void processImReady(MemoryStream* outPacket, const Session& session);

private:
	TCPSocketPtr mListenSocket;
	vector<Session> mSessions;
	array<bool, NUM_MAX_PLAYER> mUserReadied;
	map<int, string> mIdToNickname;

	bool mbFullUsers;
	int mNumCurUsers;
};