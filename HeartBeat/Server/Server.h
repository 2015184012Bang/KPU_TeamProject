#pragma once

#include <thread>

#include "Game.h"

constexpr int NUM_MAX_PLAYER = 1;

class Server : public Game
{
	struct Connection
	{
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
	void waitPlayers();
	void processPacket(MemoryStream* outPacket);

private:
	vector<Connection> mConnections;
};