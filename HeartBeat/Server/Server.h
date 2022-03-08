#pragma once

#include "Game.h"

class Server : public Game
{
public: 
	Server();

	virtual bool Init() override;
	virtual void Shutdown() override;
	virtual void Run() override;

	void SendPacket(const OutputMemoryBitStream& inOutputStream, const TCPSocketPtr& target);
	int RecvPacket(InputMemoryBitStream& inInputSteram, const TCPSocketPtr& tartget);

private:
	
	vector<TCPSocketPtr> mClientSockets;
	vector<InputMemoryBitStream> mPackets;
};