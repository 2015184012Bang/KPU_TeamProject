#pragma once

class GameManager;

class EnemySystem
{
public:
	EnemySystem(shared_ptr<GameManager>&& gm);

	void Update();

	void LoadStageFile(string_view fileName);

	void SetGenerate(bool value) { mbGenerate = value; }

private:
	void readStageFile(string_view fileName);

private:
	shared_ptr<GameManager> mGameManager = nullptr;

	bool mbGenerate = false;
};

