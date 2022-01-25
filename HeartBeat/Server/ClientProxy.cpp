#include "ServerPCH.h"

ClientProxy::ClientProxy(const SocketAddress& inSocketAddress, const string& inName, int inPlayerId) :
	mSocketAddress(inSocketAddress),
	mName(inName),
	mPlayerId(inPlayerId)
{
	UpdatelastPcketTime();

}

void ClientProxy::UpdatelastPcketTime()
{
	mLastPacketFromClientTime = Timing::sInstance.GetTimef();
}
