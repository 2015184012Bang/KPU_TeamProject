#pragma once

#include <queue>

#include "HeartBeat/Game.h"
#include "HeartBeat/Define.h"

using std::queue;

class EnemyGenerator;
class CollisionChecker;

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

	shared_ptr<CollisionChecker> GetCollisionChecker() { return mCollisionChecker; }

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
	void makeCollisionPacket();

	void sendToAllSessions(const MemoryStream& packet);

private:
	TCPSocketPtr mListenSocket;
	vector<Session> mSessions;
	array<bool, MAX_PLAYER> mUserReadied;
	map<int, string> mIdToNickname;

	queue<MemoryStream*> mSendQueue;

	int mNumCurUsers;

	shared_ptr<EnemyGenerator> mEnemyGenerator;
	shared_ptr<CollisionChecker> mCollisionChecker;
};