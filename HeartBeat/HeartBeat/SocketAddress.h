#pragma once

class SocketAddress
{
public:
	SocketAddress()
	{
		GetAsSockAddrIn()->sin_family = AF_INET;
		GetIP4Ref() = INADDR_ANY;
		GetAsSockAddrIn()->sin_port = 0;
	}

	SocketAddress(uint32 inAddress, uint16 inPort)
	{
		GetAsSockAddrIn()->sin_family = AF_INET;
		GetIP4Ref() = htonl(inAddress);
		GetAsSockAddrIn()->sin_port = htons(inPort);
	}

	SocketAddress(const sockaddr& inSockAddr)
	{
		memcpy(&mSockAddr, &inSockAddr, sizeof(sockaddr));
	}

	bool operator==(const SocketAddress& inOther) const
	{
		return (mSockAddr.sa_family == AF_INET &&
			GetAsSockAddrIn()->sin_port == inOther.GetAsSockAddrIn()->sin_port) &&
			(GetIP4Ref() == inOther.GetIP4Ref());
	}

	uint32 GetSize() const { return sizeof(sockaddr); }
	string ToString() const;

private:
	friend class TCPSocket;

	sockaddr mSockAddr;

	uint32& GetIP4Ref() { return *reinterpret_cast<uint32*>(&GetAsSockAddrIn()->sin_addr.S_un.S_addr); }
	const uint32 GetIP4Ref() const { return *reinterpret_cast<const uint32*> (&GetAsSockAddrIn()->sin_addr.S_un.S_addr); }


	sockaddr_in* GetAsSockAddrIn() { return reinterpret_cast<sockaddr_in*>(&mSockAddr); }
	const sockaddr_in* GetAsSockAddrIn() const { return reinterpret_cast<const sockaddr_in*>(&mSockAddr); }
};

typedef shared_ptr<SocketAddress> SocketAddressPtr;
