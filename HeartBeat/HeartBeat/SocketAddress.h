#pragma once

using SocketAddressPtr = shared_ptr<class SocketAddress>;

class SocketAddress
{
	friend class TCPSocket;

public:
	SocketAddress()
	{
		ZeroMemory(&mSockAddr, sizeof(mSockAddr));

		GetAsSockAddrIn()->sin_family = AF_INET;
		GetAsSockAddrIn()->sin_port = 0;
		GetAsSockAddrIn()->sin_addr.s_addr = INADDR_ANY;
	}

	SocketAddress(const string& address, uint16 port)
	{
		ZeroMemory(&mSockAddr, sizeof(mSockAddr));

		GetAsSockAddrIn()->sin_family = AF_INET;
		GetAsSockAddrIn()->sin_port = htons(port);
		GetAsSockAddrIn()->sin_addr.s_addr = inet_addr(address.c_str());
	}

	SocketAddress(uint16 port)
	{
		ZeroMemory(&mSockAddr, sizeof(mSockAddr));

		GetAsSockAddrIn()->sin_family = AF_INET;
		GetAsSockAddrIn()->sin_port = htons(port);
		GetAsSockAddrIn()->sin_addr.s_addr = INADDR_ANY;
	}

	uint32 GetSize() const { return sizeof(mSockAddr); }

	string ToString() const;

private:
	sockaddr_in* GetAsSockAddrIn() { return reinterpret_cast<sockaddr_in*>(&mSockAddr); }
	const sockaddr_in* GetAsSockAddrIn() const { return reinterpret_cast<const sockaddr_in*>(&mSockAddr); }

private:
	sockaddr mSockAddr;
};