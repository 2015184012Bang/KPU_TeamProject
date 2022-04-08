#include "ClientPCH.h"
#include "Client.h"

#include "HeartBeat/Tags.h"

#include "Animation.h"
#include "ClientComponents.h"
#include "ClientSystems.h"
#include "Input.h"
#include "LoginScene.h"
#include "Mesh.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Script.h"
#include "Text.h"
#include "GameScene.h"

Client::Client()
	: Game()
	, mActiveScene(nullptr)
	, mMySocket(nullptr)
	, mClientID(-1)
{

}

bool Client::Init()
{
	Input::Init();
	Timer::Init();
	SocketUtil::Init();

	mMySocket = SocketUtil::CreateTCPSocket();

	mRenderer = std::make_unique<Renderer>();
	mRenderer->Init();

	createCameraEntity();
	createAnimationTransitions();

	mActiveScene = std::make_unique<LoginScene>(this);
	mActiveScene->Enter();

	return true;
}

void Client::Shutdown()
{
	if (mActiveScene)
	{
		mActiveScene->Exit();
		mActiveScene = nullptr;
	}

	GetRegistry().clear();
	GetAllEntities().clear();

	mMySocket = nullptr;

	mRenderer->Shutdown();
	SocketUtil::Shutdown();
}

void Client::Run()
{
	processInput();
	update();
	render();
}

void Client::ChangeScene(Scene* scene)
{
	if (scene == nullptr)
	{
		return;
	}

	mActiveScene->Exit();
	mActiveScene = nullptr;
	mActiveScene = unique_ptr<Scene>(scene);
	mActiveScene->Enter();
}

Entity Client::CreateSkeletalMeshEntity(const wstring& meshFile, const wstring& texFile, const wstring& skelFile, const wstring& boxFile)
{
	Entity e = Entity(getNewEntt(), this);

	auto& transform = e.AddComponent<TransformComponent>();
	e.AddTag<Tag_SkeletalMesh>();
	auto& id = e.AddComponent<IDComponent>();
	e.AddComponent<MeshRendererComponent>(ResourceManager::GetMesh(meshFile), ResourceManager::GetTexture(texFile));
	e.AddComponent<AnimatorComponent>(ResourceManager::GetSkeleton(skelFile));

	RegisterEntity(id.ID, e);

	if (boxFile.size() != 0)
	{
		e.AddComponent<BoxComponent>(ResourceManager::GetAABB(boxFile), transform.Position, transform.Rotation.y);
		e.AddComponent<DebugDrawComponent>(ResourceManager::GetDebugMesh(boxFile));
	}

	return e;
}

Entity Client::CreateSkeletalMeshEntity(const wstring& meshFile, const wstring& texFile, const wstring& skelFile, const uint64 eid, const wstring& boxFile /*= L""*/)
{
	Entity e = Entity(getNewEntt(), this);

	auto& transform = e.AddComponent<TransformComponent>();
	e.AddTag<Tag_SkeletalMesh>();
	auto& id = e.AddComponent<IDComponent>(eid);
	e.AddComponent<MeshRendererComponent>(ResourceManager::GetMesh(meshFile), ResourceManager::GetTexture(texFile));
	e.AddComponent<AnimatorComponent>(ResourceManager::GetSkeleton(skelFile));

	RegisterEntity(id.ID, e);

	if (boxFile.size() != 0)
	{
		e.AddComponent<BoxComponent>(ResourceManager::GetAABB(boxFile), transform.Position, transform.Rotation.y);
		e.AddComponent<DebugDrawComponent>(ResourceManager::GetDebugMesh(boxFile));
	}

	return e;
}

Entity Client::CreateStaticMeshEntity(const wstring& meshFile, const wstring& texFile, const wstring& boxFile)
{
	Entity e = Entity(getNewEntt(), this);

	auto& transform = e.AddComponent<TransformComponent>();
	e.AddTag<Tag_StaticMesh>();
	auto& id = e.AddComponent<IDComponent>();
	e.AddComponent<MeshRendererComponent>(ResourceManager::GetMesh(meshFile), ResourceManager::GetTexture(texFile));

	RegisterEntity(id.ID, e);

	if (boxFile.size() != 0)
	{
		e.AddComponent<BoxComponent>(ResourceManager::GetAABB(boxFile), transform.Position, transform.Rotation.y);
		e.AddComponent<DebugDrawComponent>(ResourceManager::GetDebugMesh(boxFile));
	}

	return e;
}

Entity Client::CreateStaticMeshEntity(const wstring& meshFile, const wstring& texFile, const uint64 eid)
{
	Entity e = Entity(getNewEntt(), this);

	auto& transform = e.AddComponent<TransformComponent>();
	e.AddTag<Tag_StaticMesh>();
	e.AddComponent<MeshRendererComponent>(ResourceManager::GetMesh(meshFile), ResourceManager::GetTexture(texFile));
	auto& id = e.AddComponent<IDComponent>(eid);
	RegisterEntity(id.ID, e);

	return e;
}

Entity Client::CreateSpriteEntity(int width, int height, const wstring& texFile, int drawOrder /*= 100*/)
{
	Entity e = Entity(getNewEntt(), this);

	e.AddTag<Tag_Sprite>();
	auto& id = e.AddComponent<IDComponent>();
	e.AddComponent<RectTransformComponent>(width, height);
	e.AddComponent<SpriteRendererComponent>(new SpriteMesh(width, height), ResourceManager::GetTexture(texFile), drawOrder);

	RegisterEntity(id.ID, e);

	// Do sorting by draw order when sprite is added
	GetRegistry().sort<SpriteRendererComponent>([](const auto& lhs, const auto& rhs)
		{
			return lhs.DrawOrder < rhs.DrawOrder;
		});

	return e;
}

Entity Client::CreateTextEntity(const wstring& fontFile)
{
	Entity e = Entity(getNewEntt(), this);

	e.AddTag<Tag_Text>();
	auto& id = e.AddComponent<IDComponent>();
	e.AddComponent<RectTransformComponent>(0, 0);
	e.AddComponent<TextComponent>(new Text(ResourceManager::GetFont(fontFile)));

	RegisterEntity(id.ID, e);

	return e;
}

void Client::processInput()
{
	Input::Update();

	if (Input::IsButtonPressed(eKeyCode::Escape))
	{
		SetRunning(false);
	}

	processButton();

	if (mActiveScene)
	{
		mActiveScene->ProcessInput();
	}
}

void Client::update()
{
	Timer::Update();

	float deltaTime = Timer::GetDeltaTime();

	updateScript(deltaTime);
	updateAnimation(deltaTime);
	updateCollisionBox(deltaTime);

	if (mActiveScene)
	{
		mActiveScene->Update(deltaTime);
	}
}

void Client::render()
{
	mRenderer->BeginRender();

	// Set 3D camera
	auto& camera = mMainCamera.GetComponent<CameraComponent>();
	ClientSystems::BindViewProjectionMatrix(camera.Position,
		camera.Target, camera.Up, camera.FOV, camera.Buffer);

	drawSkeletalMesh();
	drawStaticMesh();

#ifdef _DEBUG
	drawCollisionBox();
#endif

	// Set 2D camera
	auto& spriteCamera = m2dCamera.GetComponent<CameraComponent>();
	ClientSystems::BindViewProjectionMatrixOrtho(spriteCamera.Buffer);

	drawSpriteAndText();

	mActiveScene->Render(mRenderer);

	mRenderer->EndRender();
}

void Client::createCameraEntity()
{
	mMainCamera = Entity(getNewEntt(), this);
	mMainCamera.AddComponent<CameraComponent>(Vector3(0.0f, 500.0f, -500.0f), Vector3(0.0f, 0.0f, 0.0f));
	mMainCamera.AddTag<Tag_Camera>();
	mMainCamera.AddTag<Tag_DontDestroyOnLoad>();

	m2dCamera = Entity(getNewEntt(), this);
	m2dCamera.AddComponent<CameraComponent>();
	m2dCamera.AddTag<Tag_Camera>();
	m2dCamera.AddTag<Tag_DontDestroyOnLoad>();
}

void Client::createAnimationTransitions()
{
	// 바이러스 애니메이션 트랜지션 설정
	{
		Animation* idleAnim = ResourceManager::GetAnimation(ANIM(L"Virus_Idle.anim"));
		Animation* runningAnim = ResourceManager::GetAnimation(ANIM(L"Virus_Run.anim"));
		Animation* attackingAnim = ResourceManager::GetAnimation(ANIM(L"Virus_Attack.anim"));
		attackingAnim->SetLoop(false);

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack", attackingAnim);
		runningAnim->AddTransition("Idle", idleAnim);
		attackingAnim->AddTransition("WhenEnd", idleAnim);
	}

	// 캐릭터_그린
	{
		Animation* idleAnim = ResourceManager::GetAnimation(ANIM(L"CG_Idle.anim"));
		Animation* runningAnim = ResourceManager::GetAnimation(ANIM(L"CG_Run.anim"));
		idleAnim->AddTransition("Run", runningAnim);
		runningAnim->AddTransition("Idle", idleAnim);
	}

	// 캐릭터_핑크
	{
		Animation* idleAnim = ResourceManager::GetAnimation(ANIM(L"CP_Idle.anim"));
		Animation* runningAnim = ResourceManager::GetAnimation(ANIM(L"CP_Run.anim"));
		idleAnim->AddTransition("Run", runningAnim);
		runningAnim->AddTransition("Idle", idleAnim);
	}

	// 캐릭터_레드
	{
		Animation* idleAnim = ResourceManager::GetAnimation(ANIM(L"CR_Idle.anim"));
		Animation* runningAnim = ResourceManager::GetAnimation(ANIM(L"CR_Run.anim"));
		idleAnim->AddTransition("Run", runningAnim);
		runningAnim->AddTransition("Idle", idleAnim);
	}

	// NPC(세포)
	{
		Animation* idleAnim = ResourceManager::GetAnimation(ANIM(L"Cell_Idle.anim"));
		Animation* runningAnim = ResourceManager::GetAnimation(ANIM(L"Cell_Run.anim"));
		Animation* attackingAnim = ResourceManager::GetAnimation(ANIM(L"Cell_Attack.anim"));

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack", attackingAnim);
		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack", attackingAnim);
	}
}

void Client::processButton()
{
	if (Input::IsButtonPressed(eKeyCode::MouseLButton))
	{
		auto view = (GetRegistry()).view<ButtonComponent>();

		for (auto entity : view)
		{
			Entity e = Entity(entity, this);

			auto& rectTransform = e.GetComponent<RectTransformComponent>();

			bool contains = ClientSystems::Intersects(rectTransform.Position, rectTransform.Width, rectTransform.Height);

			if (contains)
			{
				auto& button = e.GetComponent<ButtonComponent>();
				button.CallbackFunc();
				break;
			}
		}
	}
}

void Client::updateScript(float deltaTime)
{
	auto view = GetRegistry().view<ScriptComponent>();

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

void Client::updateAnimation(float deltaTime)
{
	auto view = GetRegistry().view<AnimatorComponent>();
	for (auto entity : view)
	{
		auto& animator = view.get<AnimatorComponent>(entity);

		ClientSystems::UpdateAnimation(&animator, deltaTime);
	}
}

void Client::updateCollisionBox(float deltaTime)
{
	auto view = GetRegistry().view<BoxComponent>();
	for (auto entity : view)
	{
		Entity e = Entity(entity, this);

		TransformComponent& transform = e.GetComponent<TransformComponent>();
		BoxComponent& box = e.GetComponent<BoxComponent>();

		ClientSystems::UpdateBox(box.Local, &box.World, transform.Position, transform.Rotation.y, transform.bDirty);
	}
}

void Client::drawSkeletalMesh()
{
	gCmdList->SetPipelineState(mRenderer->GetSkeletalMeshPSO().Get());
	auto view = GetRegistry().view<Tag_SkeletalMesh>();
	for (auto entity : view)
	{
		Entity e = Entity(entity, this);

		TransformComponent& transform = e.GetComponent<TransformComponent>();
		ClientSystems::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);

		AnimatorComponent& animator = e.GetComponent<AnimatorComponent>();
		ClientSystems::BindBoneMatrix(animator.Palette, animator.Buffer);

		MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();
		mRenderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
	}
}

void Client::drawStaticMesh()
{
	gCmdList->SetPipelineState(mRenderer->GetStaticMeshPSO().Get());

	{
		auto view = GetRegistry().view<Tag_StaticMesh>(entt::exclude<AttachmentChildComponent>);
		for (auto entity : view)
		{
			Entity e = Entity(entity, this);

			TransformComponent& transform = e.GetComponent<TransformComponent>();
			ClientSystems::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);
			MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();
			mRenderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
		}
	}

	{
		auto view = GetRegistry().view<AttachmentChildComponent>();
		for (auto entity : view)
		{
			Entity e = Entity(entity, this);

			TransformComponent& transform = e.GetComponent<TransformComponent>();
			AttachmentChildComponent& attach = e.GetComponent<AttachmentChildComponent>();
			ClientSystems::BindWorldMatrixAttached(&transform, &attach);
			MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();
			mRenderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
		}
	}
}

void Client::drawCollisionBox()
{
	gCmdList->SetPipelineState(mRenderer->GetWireframePSO().Get());
	auto view = GetRegistry().view<DebugDrawComponent>();
	for (auto entity : view)
	{
		Entity e = Entity(entity, this);

		TransformComponent& transform = e.GetComponent<TransformComponent>();
		ClientSystems::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);

		DebugDrawComponent& debug = e.GetComponent<DebugDrawComponent>();
		mRenderer->SubmitDebugMesh(debug.Mesi);
	}
}

void Client::drawSpriteAndText()
{
	gCmdList->SetPipelineState(mRenderer->GetSpritePSO().Get());

	// Draw sprite
	{
		auto view = GetRegistry().view<SpriteRendererComponent>();

		for (auto entity : view)
		{
			Entity e = Entity(entity, this);

			RectTransformComponent& rect = e.GetComponent<RectTransformComponent>();
			ClientSystems::BindWorldMatrix(rect.Position, &rect.Buffer, &rect.bDirty);

			SpriteRendererComponent& spriteRenderer = e.GetComponent<SpriteRendererComponent>();
			mRenderer->SubmitSprite(spriteRenderer.Mesi, spriteRenderer.Tex);
		}
	}

	// Draw text
	{
		auto view = GetRegistry().view<Tag_Text>();

		for (auto entity : view)
		{
			Entity e = Entity(entity, this);

			RectTransformComponent& rect = e.GetComponent<RectTransformComponent>();
			ClientSystems::BindWorldMatrix(rect.Position, &rect.Buffer, &rect.bDirty);

			TextComponent& t = e.GetComponent<TextComponent>();
			mRenderer->SubmitText(t.Txt);
		}
	}
}
