#pragma once
class ClientProxy
{
public:
	ClientProxy(const SocketAddress& inSocketAddress, const string& inName, int inPlayerId);

	const SocketAddress& GetSocketAddress() const { return mSocketAddress; }
	int GetPlayerId() const { return mPlayerId; }
	const string& GetName() const { return mName; }

	void UpdatelastPcketTime();
	float GetLastPacketFormClientTime() const { return mLastPacketFromClientTime; }


private:
	SocketAddress mSocketAddress;
	string mName;
	int mPlayerId;

	float mLastPacketFromClientTime;

};

typedef shared_ptr<ClientProxy> ClientProxyPtr;