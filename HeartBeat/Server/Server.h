#pragma once

#include <queue>

#include "HeartBeat/Game.h"

using std::queue;

constexpr int NUM_MAX_PLAYER = 1;

class EnemyGenerator;

class Server : public Game
{
	struct Session
	{
		Session(bool connect, TCPSocketPtr socket, const SocketAddress& addr, int id)
			: bConnect(connect)
			, ClientSocket(socket)
			, ClientAddr(addr)
			, ClientID(id)
			, CharacterID(0)
		{}

		bool bConnect;
		TCPSocketPtr ClientSocket;
		SocketAddress ClientAddr;
		int ClientID;
		HBID CharacterID;
	};

public:
	Server();

	virtual bool Init() override;
	virtual void Shutdown() override;
	virtual void Run() override;

	Entity CreateEntity();

	void PushPacket(MemoryStream* packet);

private:
	void acceptClients();
	void recvFromClients();
	void clearIfDisconnected();
	void flushSendQueue();

	void processPacket(MemoryStream* outPacket, const Session& session);
	void processLoginRequest(MemoryStream* outPacket, const Session& session);
	void processImReady(MemoryStream* outPacket, const Session& session);
	void processUserInput(MemoryStream* outPacket);
	
	void makeUpdateTransformPacket();
	void makeEnemyCreatePacket();

	void sendToAllSessions(const MemoryStream& packet);

private:
	TCPSocketPtr mListenSocket;
	vector<Session> mSessions;
	array<bool, NUM_MAX_PLAYER> mUserReadied;
	map<int, string> mIdToNickname;

	queue<MemoryStream*> mSendQueue;

	int mNumCurUsers;

	shared_ptr<EnemyGenerator> mEnemyGenerator;
};