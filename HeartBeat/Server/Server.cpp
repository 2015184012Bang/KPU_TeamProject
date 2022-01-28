#include "ServerPCH.h"

Server::Server()
{
}

bool Server::Init()
{
    InitNetworkManager();
    return false;
}

void Server::Shutdown()
{
}

void Server::Run()
{
}

void Server::HandleNewClient()
{
}

void Server::HandleLostClient()
{
}

bool Server::InitNetworkManager()
{
    return false;
}
