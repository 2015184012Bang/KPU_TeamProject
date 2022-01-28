#include "ServerPCH.h"

NetworkManagerServer* NetworkManagerServer::sInstance;

NetworkManagerServer::NetworkManagerServer() :
	mNewPlayerId(1),
	mNewNetworkId(1),
	mTimeBetweenStatePackets(0.033f),
	mClientDisconnectTimeout(3.f)
{
}

bool NetworkManagerServer::StaticInit(uint16 inPort)
{
	sInstance = new NetworkManagerServer();
	return sInstance->Init(inPort);
}

void NetworkManagerServer::ProcessPacket(InputMemoryBitStream& inInputStream, const SocketAddress& inFromAddress)
{
	auto it = mAddressToClientMap.find(inFromAddress);
	if (it == mAddressToClientMap.end())
	{
		HandlePacketFromNewClient(inInputStream, inFromAddress);
	}
	else
	{
		ProcessPacket((*it).second, inInputStream);
	}
}

void NetworkManagerServer::HandleConnectionReset(const SocketAddress& inFromAddress)
{
	auto it = mAddressToClientMap.find(inFromAddress);
	if (it != mAddressToClientMap.end())
	{
		HandleClientDisconnected(it->second);
	}
}

void NetworkManagerServer::SendOutgoingPackets()
{
	for (auto it = mAddressToClientMap.begin(), end = mAddressToClientMap.end(); it != end; ++it)
	{
		ClientProxyPtr clientProxy = it->second;
		//if (clientProxy->IsLastMoveTimestampDirty())
		//{
		//	SendStatePacketToClient(clientProxy);
		//}
	}
}

void NetworkManagerServer::CheckForDisconnects()
{
}

ClientProxyPtr NetworkManagerServer::GetClientProxy(int inPlayerId) const
{
	return ClientProxyPtr();
}

void NetworkManagerServer::HandlePacketFromNewClient(InputMemoryBitStream& inInputStream, const SocketAddress& inFromAddress)
{
	//uint32	packetType;
	//inInputStream.Read(packetType);
	//if (packetType == kHelloCC)
	//{
	//	//read the name
	//	string name;
	//	inInputStream.Read(name);
	//	ClientProxyPtr newClientProxy = std::make_shared< ClientProxy >(inFromAddress, name, mNewPlayerId++);
	//	mAddressToClientMap[inFromAddress] = newClientProxy;
	//	mPlayerIdToClientMap[newClientProxy->GetPlayerId()] = newClientProxy;

	//	//tell the server about this client, spawn a cat, etc...
	//	//if we had a generic message system, this would be a good use for it...
	//	//instead we'll just tell the server directly
	//	static_cast<Server*> (Engine::sInstance.get())->HandleNewClient(newClientProxy);

	//	//and welcome the client...
	//	SendWelcomePacket(newClientProxy);

	//	//and now init the replication manager with everything we know about!
	//	for (const auto& pair : mNetworkIdToGameObjectMap)
	//	{
	//		newClientProxy->GetReplicationManagerServer().ReplicateCreate(pair.first, pair.second->GetAllStateMask());
	//	}
	//}
	//else
	//{
	//	//bad incoming packet from unknown client- we're under attack!!
	//	LOG("Bad incoming packet from unknown client at socket %s", inFromAddress.ToString().c_str());
	//}
}

void NetworkManagerServer::ProcessPacket(ClientProxyPtr inClientProxy, InputMemoryBitStream& inInputStream)
{
	inClientProxy->UpdatelastPcketTime();

	uint32 packetType;
	inInputStream.Read(packetType);
	switch (packetType)
	{
	case kHelloCC:
		SendWelcomePacket(inClientProxy);
		break;
	case kInputCC:
		HandleInputPacket(inClientProxy, inInputStream);
		break;
	default:
		HB_LOG("Unknown packet type received from %s", inClientProxy->GetSocketAddress().ToString().c_str());
		break;
	}
}

void NetworkManagerServer::SendWelcomePacket(ClientProxyPtr inClientProxy)
{
	OutputMemoryBitStream welcomePacket;

	welcomePacket.Write(kWelcomeCC);
	welcomePacket.Write(inClientProxy->GetPlayerId());

	HB_LOG("Server Welcoming, new client '%s' as player %d", inClientProxy->GetName().c_str(), inClientProxy->GetPlayerId());

	SendPacket(welcomePacket, inClientProxy->GetSocketAddress());
}

void NetworkManagerServer::UpdateAllClients()
{
}

void NetworkManagerServer::HandleInputPacket(ClientProxyPtr inClientProxy, InputMemoryBitStream& inInputStream)
{

}

void NetworkManagerServer::HandleClientDisconnected(ClientProxyPtr inClientProxy)
{
}

int NetworkManagerServer::GetNewNetworkId()
{
	return 0;
}
