#include "PCH.h"

string SocketAddress::ToString() const
{
	stringstream ss;

	ss << inet_ntoa(GetAsSockAddrIn()->sin_addr) << ":" << ntohs(GetAsSockAddrIn()->sin_port);

	return ss.str();
}
