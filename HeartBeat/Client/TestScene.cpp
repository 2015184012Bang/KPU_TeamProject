#include "ClientPCH.h"
#include "TestScene.h"

#include "Input.h"
#include "Client.h"

#include "Renderer.h"
#include "ResourceManager.h"
#include "ClientSystems.h"

#include "CharacterMovement.h"

TestScene* TestScene::sInstance;

TestScene::TestScene(Client* owner)
	: Scene(owner)
{

}

void TestScene::StaticCreate(Client* owner)
{
	static TestScene instance(owner);
	sInstance = &instance;
}

TestScene* TestScene::Get()
{
	return sInstance;
}

void TestScene::Enter()
{
	HB_LOG("TestScene::Enter");

	{
		mTestEntity = Entity(mOwner->CreateEntity(), mOwner);
		mTestEntity.AddComponent<MeshRendererComponent>(ResourceManager::GetMesh(L"Assets/Meshes/t4.mesh"),
			ResourceManager::GetTexture(L"Assets/Textures/tt.png"));
		auto& transform = mTestEntity.AddComponent<TransformComponent>();
		//transform.Scale = 10.0f;
		mTestEntity.AddTag<SkeletalMesh>();
		auto& animator = mTestEntity.AddComponent<AnimatorComponent>(ResourceManager::GetSkeleton(L"Assets/Skeletons/t4.skel"));
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(L"Assets/Animations/t4.anim"), 1.0f, true);

		mTestEntity.AddComponent<BoxComponent>(ResourceManager::GetAABB(L"Assets/Boxes/Knight.box"), transform.Position, transform.Rotation.y);

		mTestEntity.AddComponent<DebugDrawComponent>(ResourceManager::GetDebugMesh(L"Assets/Boxes/Knight.box"));

		mTestEntity.AddComponent<ScriptComponent>(new CharacterMovement(mTestEntity));
	}

	{
		mKnight = Entity(mOwner->CreateEntity(), mOwner);
		mKnight.AddComponent<MeshRendererComponent>(ResourceManager::GetMesh(L"Assets/Meshes/Knight.mesh"),
			ResourceManager::GetTexture(L"Assets/Textures/Knight.png"));
		auto& transform = mKnight.AddComponent<TransformComponent>();
		transform.Position = Vector3(500.0f, 0.0f, 0.0f);
		mKnight.AddTag<SkeletalMesh>();
		auto& animator = mKnight.AddComponent<AnimatorComponent>(ResourceManager::GetSkeleton(L"Assets/Skeletons/Knight.skel"));
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(L"Assets/Animations/Knight_Run.anim"), 1.0f, true);

		mKnight.AddComponent<BoxComponent>(ResourceManager::GetAABB(L"Assets/Boxes/Knight.box"), transform.Position, transform.Rotation.y);

		mKnight.AddComponent<DebugDrawComponent>(ResourceManager::GetDebugMesh(L"Assets/Boxes/Knight.box"));
	}

	mMainCamera = Entity(mOwner->CreateEntity(), mOwner);
	mMainCamera.AddComponent<CameraComponent>(Vector3(0.0f, 500.0f, -500.0f), Vector3(0.0f, 0.0f, 0.0f));
	mMainCamera.AddTag<Camera>();
}

void TestScene::Exit()
{
	HB_LOG("TestScene::Exit");
}

void TestScene::ProcessInput()
{

}

void TestScene::Update(float deltaTime)
{
	{
		auto view = (mOwner->GetRegistry()).view<ScriptComponent>();

		for (auto entity : view)
		{
			auto& script = view.get<ScriptComponent>(entity);

			if (!script.bInitialized)
			{
				script.NativeScript->Start();
				script.bInitialized = true;
			}

			script.NativeScript->Update(deltaTime);
		}
	}

	auto& transform = mTestEntity.GetComponent<TransformComponent>();
	ClientSystems::RotateY(&transform.Rotation, 30.0f, deltaTime, &transform.bDirty);

	{
		auto view = (mOwner->GetRegistry()).view<AnimatorComponent>();
		for (auto entity : view)
		{
			auto& animator = view.get<AnimatorComponent>(entity);

			ClientSystems::UpdateAnimation(animator.Anim, animator.Skel,
				&animator.AnimTime, animator.AnimPlayRate, animator.bLoop, &animator.Palette, deltaTime);
		}
	}

	{
		auto view = (mOwner->GetRegistry()).view<BoxComponent>();
		for (auto entity : view)
		{
			Entity e = Entity(entity, mOwner);

			TransformComponent& transform = e.GetComponent<TransformComponent>();
			BoxComponent& box = e.GetComponent<BoxComponent>();

			ClientSystems::UpdateBox(box.Local, &box.World, transform.Position, transform.Rotation.y, transform.bDirty);
		}
	}

	{
		auto& box1 = mTestEntity.GetComponent<BoxComponent>();
		auto& box2 = mKnight.GetComponent<BoxComponent>();

		bool collides = ClientSystems::Intersects(box1.World, box2.World);

		if (collides)
		{
			HB_LOG("Collision dectected!!");
		}
	}
}

void TestScene::Render(unique_ptr<Renderer>& renderer)
{
	auto& camera = mMainCamera.GetComponent<CameraComponent>();
	ClientSystems::BindViewProjectionMatrix(camera.Position,
		camera.Target, camera.Up, camera.FOV, camera.Buffer);

	{
		gCmdList->SetPipelineState(renderer->GetSkeletalMeshPSO().Get());
		auto view = (mOwner->GetRegistry()).view<SkeletalMesh>();
		for (auto entity : view)
		{
			Entity e = Entity(entity, mOwner);

			TransformComponent& transform = e.GetComponent<TransformComponent>();
			ClientSystems::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, transform.Buffer, &transform.bDirty);

			AnimatorComponent& animator = e.GetComponent<AnimatorComponent>();
			ClientSystems::BindBoneMatrix(animator.Palette, animator.Buffer);

			MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();
			renderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
		}
	}

	{
		gCmdList->SetPipelineState(renderer->GetStaticMeshPSO().Get());
		auto view = (mOwner->GetRegistry()).view<StaticMesh>();
		for (auto entity : view)
		{
			Entity e = Entity(entity, mOwner);

			TransformComponent& transform = e.GetComponent<TransformComponent>();
			ClientSystems::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, transform.Buffer, &transform.bDirty);

			MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();
			renderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
		}
	}

#ifdef _DEBUG
	{
		gCmdList->SetPipelineState(renderer->GetWireframePSO().Get());
		auto view = (mOwner->GetRegistry()).view<DebugDrawComponent>();
		for (auto entity : view)
		{
			Entity e = Entity(entity, mOwner);

			TransformComponent& transform = e.GetComponent<TransformComponent>();
			ClientSystems::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, transform.Buffer, &transform.bDirty);

			DebugDrawComponent& debug = e.GetComponent<DebugDrawComponent>();
			renderer->SubmitDebugMesh(debug.Mesi);
		}
	}
#endif
}
