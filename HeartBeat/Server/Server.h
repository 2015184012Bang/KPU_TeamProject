#pragma once

#include <thread>

#include "Game.h"

class Server : public Game
{
public:
	Server();

	virtual bool Init() override;
	virtual void Shutdown() override;
	virtual void Run() override;

private:
	void waitPlayers();
	void clientThreadFunc(const TCPSocketPtr& clientSocket, int clientNum);

private:
	static const int MAX_PLAYER_NUM = 1;

	vector<std::thread> mClientThreads;
	vector<TCPSocketPtr> mClientSockets;
	deque<MemoryStream> mPackets;

	CRITICAL_SECTION mCS;
};