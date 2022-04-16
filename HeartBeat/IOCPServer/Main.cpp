#include "Network/IOCPServer.h"


int main()
{
	IOCPServer server;

	server.Init();
	server.BindAndListen(9000);
	server.StartServer(3);
	
	while (true);

	server.End();
}