#pragma once
class NetworkManger
{
public:
	static const uint32 kHelloCC = 'HELO';
	static const uint32 kWelcomeCC = 'WLCM';
	static const uint32 kStateCC = 'STAT';
	static const uint32 kInputCC = 'INPT';
	static const int kMaxPacketsPerFrameCount = 10;

	NetworkManger();
	virtual ~NetworkManger();

	bool Init(uint16 inPort);
	void ProcessIncomingPackets();

	


private:

};