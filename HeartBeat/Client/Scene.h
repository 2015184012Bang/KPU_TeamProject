#pragma once

class Client;

class Scene
{
public:
	Scene(Client* owner)
		: mOwner(owner) {}
	virtual ~Scene() = default;

	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	virtual void Enter() = 0;
	virtual void Exit() = 0;
	virtual void ProcessInput() {}
	virtual void Update(float deltaTime) {}
	virtual void Render() {}

protected:
	Client* mOwner;
};

