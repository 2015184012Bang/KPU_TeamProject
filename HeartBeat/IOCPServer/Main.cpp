#include "pch.h"
#include "GameServer.h"

int main()
{
	GameServer server;

	server.Init();
	server.BindAndListen();
	server.Run(6);
	
	while (true);

	server.End();
}