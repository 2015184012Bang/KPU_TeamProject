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

	//{
	//	mClientSocket = SocketUtil::CreateTCPSocket();
	//	SocketAddress serveraddr("127.0.0.1", SERVER_PORT);
	//	
	//	int retVal = SOCKET_ERROR;
	//	do 
	//	{
	//		retVal = mClientSocket->Connect(serveraddr);
	//	} while (retVal == SOCKET_ERROR);

	//	mClientSocket->SetNonBlockingMode(true);
	//}

	{
		mSprite = mOwner->CreateSpriteEntity(100, 100, L"Assets/Textures/Smile.png");
		mSprite2 = mOwner->CreateSpriteEntity(100, 100, L"Assets/Textures/21_HEnemy.png");

		auto& spriteRenderer = mSprite2.GetComponent<SpriteRendererComponent>();
		spriteRenderer.DrawOrder = 10;

		auto& rect = mSprite2.GetComponent<RectTransformComponent>();
		rect.Position.x = 50.0f;

		// Sort SpriteRendererComponent when sprite adds.
		(mOwner->GetRegistry()).sort<SpriteRendererComponent>([](const auto& lhs, const auto& rhs)
			{
				return lhs.DrawOrder < rhs.DrawOrder;
			});
	}

	{
		mEnemy = mOwner->CreateSkeletalMeshEntity(L"Assets/Meshes/21_HEnemy.mesh", L"Assets/Textures/21_HEnemy.png",
			L"Assets/Skeletons/21_HEnemy.skel", L"Assets/Boxes/21_HEnemy.box");

		auto& animator = mEnemy.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(L"Assets/Animations/924_Running.anim"), 1.0f);

		Animation* idleAnim = ResourceManager::GetAnimation(L"Assets/Animations/921_Idle.anim");
		idleAnim->AddTransition("Walking", ResourceManager::GetAnimation(L"Assets/Animations/924_Running.anim"));
		idleAnim->AddTransition("Attacking", ResourceManager::GetAnimation(L"Assets/Animations/922_Attacking.anim"));

		Animation* runningAnim = ResourceManager::GetAnimation(L"Assets/Animations/924_Running.anim");
		runningAnim->AddTransition("Idle", ResourceManager::GetAnimation(L"Assets/Animations/921_Idle.anim"));

		Animation* attackingAnim = ResourceManager::GetAnimation(L"Assets/Animations/922_Attacking.anim");
		attackingAnim->SetLoop(false);
		attackingAnim->AddTransition("WhenEnd", ResourceManager::GetAnimation(L"Assets/Animations/921_Idle.anim"));

		mEnemy.AddComponent<ScriptComponent>(new CharacterMovement(mEnemy));
	}

	{
		mCell = mOwner->CreateSkeletalMeshEntity(L"Assets/Meshes/11_Cell.mesh", L"Assets/Textures/11_Cell_Red.png",
			L"Assets/Skeletons/11_Cell.skel", L"Assets/Boxes/11_Cell.box");

		auto& animator = mCell.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(L"Assets/Animations/912_Running.anim"), 1.0f);

		auto& transform = mCell.GetComponent<TransformComponent>();
		transform.Position.x = 300.0f;
		transform.Rotation.y = 180.0f;
	}

	{
		mPlayer = mOwner->CreateSkeletalMeshEntity(L"Assets/Meshes/03_Character_Pink.mesh", L"Assets/Textures/03_Character_Pink.png",
			L"Assets/Skeletons/03_Character_Pink.skel", L"Assets/Boxes/01_Character.box");

		auto& animator = mPlayer.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(L"Assets/Animations/901_Idle_Pink.anim"), 1.0f);

		auto& transform = mPlayer.GetComponent<TransformComponent>();
		transform.Position.x = -300.0f;
	}

	mMainCamera = Entity(mOwner->CreateEntity(), mOwner);
	mMainCamera.AddComponent<CameraComponent>(Vector3(0.0f, 500.0f, -500.0f), Vector3(0.0f, 0.0f, 0.0f));
	mMainCamera.AddTag<Camera>();

	m2dCamera = Entity(mOwner->CreateEntity(), mOwner);
	m2dCamera.AddComponent<CameraComponent>();
	m2dCamera.AddTag<Camera>();
}

void TestScene::Exit()
{
	HB_LOG("TestScene::Exit");

	//mClientSocket = nullptr;
}

void TestScene::ProcessInput()
{
	if (Input::IsButtonPressed(eKeyCode::MouseRButton))
	{
		auto view = (mOwner->GetRegistry()).view<RectTransformComponent>();

		for (auto entity : view)
		{
			auto& rectTransform = view.get<RectTransformComponent>(entity);

			bool res = ClientSystems::Intersects(rectTransform.Position, rectTransform.Width, rectTransform.Height);

			if (res)
			{
				HB_LOG("UI intersects!");
			}
		}
	}
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

	{
		auto view = (mOwner->GetRegistry().view<SkeletalMesh>());
		for (auto entity : view)
		{
			Entity e = Entity(entity, mOwner);

			auto& transform = e.GetComponent<TransformComponent>();

			ClientSystems::RotateY(&transform.Rotation, 30.0f, deltaTime, &transform.bDirty);
		}
	}

	{
		auto view = (mOwner->GetRegistry()).view<AnimatorComponent>();
		for (auto entity : view)
		{
			auto& animator = view.get<AnimatorComponent>(entity);

			ClientSystems::UpdateAnimation(&animator, deltaTime);
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
		auto& box1 = mEnemy.GetComponent<BoxComponent>();
		auto& box2 = mCell.GetComponent<BoxComponent>();

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
			ClientSystems::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);

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
			ClientSystems::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);

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
			ClientSystems::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);

			DebugDrawComponent& debug = e.GetComponent<DebugDrawComponent>();
			renderer->SubmitDebugMesh(debug.Mesi);
		}
	}
#endif

	auto& spriteCamera = m2dCamera.GetComponent<CameraComponent>();
	ClientSystems::BindViewProjectionMatrixOrtho(spriteCamera.Buffer);
	{
		gCmdList->SetPipelineState(renderer->GetSpritePSO().Get());

		auto view = (mOwner->GetRegistry()).view<SpriteRendererComponent>();

		for (auto entity : view)
		{
			Entity e = Entity(entity, mOwner);

			RectTransformComponent& rect = e.GetComponent<RectTransformComponent>();
			ClientSystems::BindWorldMatrix(rect.Position, &rect.Buffer, &rect.bDirty);

			SpriteRendererComponent& spriteRenderer = e.GetComponent<SpriteRendererComponent>();
			renderer->SubmitSprite(spriteRenderer.Mesi, spriteRenderer.Tex);
		}
	}
}
