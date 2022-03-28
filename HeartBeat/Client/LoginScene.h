#pragma once
#include "Scene.h"

class LoginScene :
	public Scene
{

public:
	LoginScene(Client* owner);

	virtual void Enter() override;
	virtual void Exit() override;
	virtual void ProcessInput() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(unique_ptr<Renderer>& renderer) override;

private:
	
};

