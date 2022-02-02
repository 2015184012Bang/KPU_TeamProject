#include "ServerPCH.h"
#include "Server.h"

int main()
{
	unique_ptr<Server> server = std::make_unique<Server>();

	bool res = server->Init();

	if (res)
	{
		server->Run();
	}

	server->Shutdown();
}