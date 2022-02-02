#include "ServerPCH.h"
#include "Server.h"



Server::Server()
{

}

bool Server::Init()
{
	HB_LOG("SERVER INIT");

	return true;
}

void Server::Shutdown()
{
	HB_LOG("SERVER SHUTDOWN");
}

void Server::Run()
{
	while (true)
	{
		HB_LOG("SERVER RUN");
	}
}
