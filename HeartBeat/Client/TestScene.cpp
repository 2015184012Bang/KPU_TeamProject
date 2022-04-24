#include "ClientPCH.h"
#include "TestScene.h"

#include "Client.h"
#include "ResourceManager.h"

TestScene::TestScene(Client* owner)
	: Scene(owner)
{

}

void TestScene::Enter()
{
	Entity e = mOwner->CreateStaticMeshEntity(MESH(L"Plane.mesh"), TEXTURE(L"Heal.png"));
}

void TestScene::Exit()
{

}
