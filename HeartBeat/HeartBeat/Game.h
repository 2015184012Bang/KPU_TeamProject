#pragma once

class Game
{
public:
	virtual ~Game() = default;

	virtual bool Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Run() = 0;

	bool ShouldClose() const { return !mIsRunning; }

protected:
	bool mIsRunning = true;
};

