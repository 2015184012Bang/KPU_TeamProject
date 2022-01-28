#include "ServerPCH.h"

int main()
{
	unique_ptr<Server> myServer = std::make_unique<Server>();

	bool isOpenServer = myServer->Init();


	return 0;
}