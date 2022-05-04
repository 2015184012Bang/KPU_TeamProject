#pragma once

class Server;

class AIController
{
public:
	AIController(Server* server);
	~AIController();

	void Update();

private:
	Server* mServer;
};

