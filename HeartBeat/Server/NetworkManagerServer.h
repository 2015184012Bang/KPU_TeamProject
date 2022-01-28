#pragma once
class NetworkManagerServer : public NetworkManager
{
	static NetworkManagerServer* sInstance;

	static bool				StaticInit(uint16 inPort);

	virtual void			ProcessPacket(InputMemoryBitStream& inInputStream, const SocketAddress& inFromAddress) override;
	virtual void			HandleConnectionReset(const SocketAddress& inFromAddress) override;

	void			SendOutgoingPackets();
	void			CheckForDisconnects();

	ClientProxyPtr	GetClientProxy(int inPlayerId) const;

private:
	NetworkManagerServer();

	void	HandlePacketFromNewClient(InputMemoryBitStream& inInputStream, const SocketAddress& inFromAddress);
	void	ProcessPacket(ClientProxyPtr inClientProxy, InputMemoryBitStream& inInputStream);

	void	SendWelcomePacket(ClientProxyPtr inClientProxy);
	void	UpdateAllClients();

	void	HandleInputPacket(ClientProxyPtr inClientProxy, InputMemoryBitStream& inInputStream);

	void	HandleClientDisconnected(ClientProxyPtr inClientProxy);

	int		GetNewNetworkId();

	//typedef unordered_map< int, ClientProxyPtr >	IntToClientMap;
	//typedef unordered_map< SocketAddress, ClientProxyPtr >	AddressToClientMap;

	//AddressToClientMap		mAddressToClientMap;
	//IntToClientMap			mPlayerIdToClientMap;

	int				mNewPlayerId;
	int				mNewNetworkId;

	float			mTimeOfLastSatePacket;
	float			mTimeBetweenStatePackets;
	float			mClientDisconnectTimeout;
};