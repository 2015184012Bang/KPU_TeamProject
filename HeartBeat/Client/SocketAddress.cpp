#include "PCH.h"
#include "SocketAddress.h"

string SocketAddress::ToString() const
{
	std::stringstream ss;

	ss << inet_ntoa(GetAsSockAddrIn()->sin_addr) << ":" << ntohs(GetAsSockAddrIn()->sin_port);

	return ss.str();
}
