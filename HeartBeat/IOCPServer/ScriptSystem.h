#pragma once

class GameManager;

class ScriptSystem
{
public:
	ScriptSystem(shared_ptr<GameManager>&& gm);

	void Update();

private:
	shared_ptr<GameManager> mGameManager = nullptr;
};

