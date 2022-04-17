#pragma once
#include "Network/IOCPServer.h"

class GameServer :
    public IOCPServer
{
public:
    virtual void OnConnect(const INT32 sessionIndex) override;

    virtual void OnClose(const INT32 sessionIndex) override;

    virtual void OnRecv(const INT32 sessionIndex, const UINT32 dataSize, char* msg) override;

    void Run(const UINT32 maxSessionCount);

    virtual void End() override;
};

