#pragma once

class Client;
class Renderer;

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
	virtual void Render(unique_ptr<Renderer>& renderer) {}

protected:
	Client* mOwner;
};

