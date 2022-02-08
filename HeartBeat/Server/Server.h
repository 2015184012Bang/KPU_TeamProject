#pragma once

#include "Game.h"

class Server : public Game
{
public:
	Server();

	virtual bool Init() override;
	virtual void Shutdown() override;
	virtual void Run() override;
};