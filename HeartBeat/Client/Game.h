#pragma once

class Game
{
public:
	Game();
	virtual ~Game() = default;

	virtual bool Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Run() = 0;

	bool ShouldClose() const { return !mbRunning; }
	void SetRunning(bool value) { mbRunning = value; }

private:
	bool mbRunning;
};
