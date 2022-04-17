#include "GameServer.h"

int main()
{
	GameServer server;

	server.Init();
	server.BindAndListen(9000);
	server.StartServer(3);
	
	while (true);

	server.End();
}