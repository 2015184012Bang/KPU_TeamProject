#include "pch.h"
#include "GameServer.h"

int main()
{
	GameServer server;

	server.Init();
	server.BindAndListen();
	server.Run(3);
	
	while (true);

	server.End();
}