#include "ClientPCH.h"
#include "Client.h"

#include "tinyxml2.h"

#include "Tags.h"
#include "Animation.h"
#include "Components.h"
#include "Helpers.h"
#include "Input.h"
#include "LoginScene.h"
#include "Mesh.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Text.h"
#include "GameScene.h"
#include "PacketManager.h"
#include "Timer.h"
#include "Utils.h"
#include "Script.h"
#include "Mesh.h"
#include "Font.h"
#include "Texture.h"
#include "Skeleton.h"
#include "SoundManager.h"
#include "TestScene.h"

Client::Client()
	: Game()
{

}

bool Client::Init()
{
	Input::Init();
	Timer::Init();
	SoundManager::Init();

	loadServerSettingsFromXML("settings.xml");

	mPacketManager = std::make_unique<PacketManager>();
	mPacketManager->Init();

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

	mRenderer->Shutdown();
	mPacketManager->Shutdown();
	SoundManager::Shutdown();
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


Entity Client::CreateSkeletalMeshEntity(const Mesh* mesh, const Texture* texFile, const Skeleton* skelFile, string_view boxFile /*= ""*/)
{
	Entity e = Entity(GetNewEntity(), this);

	auto& transform = e.AddComponent<TransformComponent>();
	e.AddTag<Tag_SkeletalMesh>();
	e.AddComponent<MeshRendererComponent>(mesh, texFile);
	e.AddComponent<AnimatorComponent>(skelFile);

	if (boxFile.size() != 0)
	{
		e.AddComponent<BoxComponent>(ResourceManager::GetAABB(boxFile), transform.Position, transform.Rotation.y);
		e.AddComponent<DebugDrawComponent>(ResourceManager::GetDebugMesh(boxFile));
	}

	return e;
}

Entity Client::CreateSkeletalMeshEntity(const Mesh* mesh, const Texture* texFile, const Skeleton* skelFile, const uint32 eid, string_view boxFile /*= ""*/)
{
	Entity e = Entity(GetNewEntity(), this);

	auto& transform = e.AddComponent<TransformComponent>();
	e.AddTag<Tag_SkeletalMesh>();
	e.AddComponent<MeshRendererComponent>(mesh, texFile);
	e.AddComponent<AnimatorComponent>(skelFile);

	e.AddComponent<IDComponent>(eid);

	if (boxFile.size() != 0)
	{
		e.AddComponent<BoxComponent>(ResourceManager::GetAABB(boxFile), transform.Position, transform.Rotation.y);
		e.AddComponent<DebugDrawComponent>(ResourceManager::GetDebugMesh(boxFile));
	}

	return e;
}

Entity Client::CreateStaticMeshEntity(const Mesh* meshFile, const Texture* texFile, string_view boxFile /*= ""*/)
{
	Entity e = Entity(GetNewEntity(), this);

	auto& transform = e.AddComponent<TransformComponent>();
	e.AddTag<Tag_StaticMesh>();
	e.AddComponent<MeshRendererComponent>(meshFile, texFile);

	if (boxFile.size() != 0)
	{
		e.AddComponent<BoxComponent>(ResourceManager::GetAABB(boxFile), transform.Position, transform.Rotation.y);
		e.AddComponent<DebugDrawComponent>(ResourceManager::GetDebugMesh(boxFile));
	}

	return e;
}

Entity Client::CreateStaticMeshEntity(const Mesh* meshFile, const Texture* texFile, const uint32 eid, string_view boxFile /*= ""*/)
{
	Entity e = Entity(GetNewEntity(), this);

	auto& transform = e.AddComponent<TransformComponent>();
	e.AddTag<Tag_StaticMesh>();
	e.AddComponent<MeshRendererComponent>(meshFile, texFile);

	e.AddComponent<IDComponent>(eid);

	if (boxFile.size() != 0)
	{
		e.AddComponent<BoxComponent>(ResourceManager::GetAABB(boxFile), transform.Position, transform.Rotation.y);
		e.AddComponent<DebugDrawComponent>(ResourceManager::GetDebugMesh(boxFile));
	}

	return e;
}

Entity Client::CreateSpriteEntity(int width, int height, const Texture* texFile, int drawOrder /*= 100*/)
{
	Entity e = Entity(GetNewEntity(), this);

	e.AddTag<Tag_Sprite>();
	e.AddComponent<RectTransformComponent>(width, height);
	e.AddComponent<SpriteRendererComponent>(new SpriteMesh(width, height), texFile, drawOrder);

	// Do sorting by draw order when sprite is added
	GetRegistry().sort<SpriteRendererComponent>([](const auto& lhs, const auto& rhs)
		{
			return lhs.DrawOrder < rhs.DrawOrder;
		});

	return e;
}

Entity Client::CreateTextEntity(const Font* fontFile)
{
	Entity e = Entity(GetNewEntity(), this);

	e.AddTag<Tag_Text>();
	e.AddComponent<RectTransformComponent>(0, 0);
	e.AddComponent<TextComponent>(new Text(fontFile));

	return e;
}

void Client::RearrangeAttachment()
{
	auto view = GetRegistry().view<AttachmentParentComponent, TransformComponent, AnimatorComponent>();

	for (auto [entity, parent, transform, animator] : view.each())
	{
		for (auto [boneName, eid] : parent.Children)
		{
			Entity child = { eid, this };
			auto& attachment = child.GetComponent<AttachmentChildComponent>();

			attachment.BoneIndex = animator.Skel->GetBoneIndexByName(boneName);
			attachment.ParentPalette = &animator.Palette;
			attachment.ParentTransform = &transform;
		}
	}
}

void Client::loadServerSettingsFromXML(string_view fileName)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(fileName.data());

	HB_ASSERT(error == tinyxml2::XML_SUCCESS, "Failed to read xml file: {0}", fileName.data());

	auto root = doc.RootElement();
	
	auto elem = root->FirstChildElement("Server")->FirstChildElement("Port");
	string port = elem->GetText();
	ServerPort = std::stoi(port);

	elem = elem->NextSiblingElement();
	ServerIP = elem->GetText();
}

void Client::processInput()
{
	Input::Update();

	if (Input::IsButtonPressed(eKeyCode::Escape))
	{
		SetRunning(false);
	}

	processButton();

	// 패킷 수신
	mPacketManager->Recv();

	if (mActiveScene)
	{
		mActiveScene->ProcessInput();
	}
}

void Client::update()
{
	Timer::Update();

	float deltaTime = Timer::GetDeltaTime();

	updateMovement(deltaTime);
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
	Helpers::BindViewProjectionMatrix(camera.Position,
		camera.Target, camera.Up, camera.FOV, camera.Buffer);

	drawSkeletalMesh();
	drawStaticMesh();

#ifdef _DEBUG
	drawCollisionBox();
#endif

	// Set 2D camera
	auto& spriteCamera = m2dCamera.GetComponent<CameraComponent>();
	Helpers::BindViewProjectionMatrixOrtho(spriteCamera.Buffer);

	drawSpriteAndText();

	mActiveScene->Render(mRenderer);

	mRenderer->EndRender();
}

void Client::createCameraEntity()
{
	mMainCamera = Entity(GetNewEntity(), this);
	mMainCamera.AddComponent<CameraComponent>(Vector3(0.0f, 500.0f, -500.0f), Vector3(0.0f, 0.0f, 0.0f));
	mMainCamera.AddTag<Tag_Camera>();
	mMainCamera.AddTag<Tag_DontDestroyOnLoad>();

	m2dCamera = Entity(GetNewEntity(), this);
	m2dCamera.AddComponent<CameraComponent>();
	m2dCamera.AddTag<Tag_Camera>();
	m2dCamera.AddTag<Tag_DontDestroyOnLoad>();
}

void Client::createAnimationTransitions()
{
	// 바이러스 애니메이션 트랜지션 설정
	{
		Animation* idleAnim = ANIM("Virus_Idle.anim");
		Animation* runningAnim = ANIM("Virus_Run.anim");
		Animation* attackingAnim = ANIM("Virus_Attack.anim");
		attackingAnim->SetLoop(false);

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack", attackingAnim);
		runningAnim->AddTransition("Idle", idleAnim);
		attackingAnim->AddTransition("WhenEnd", idleAnim);
	}

	// 캐릭터_그린
	{
		Animation* idleAnim = ANIM("CG_Idle.anim");
		Animation* runningAnim = ANIM("CG_Run.anim");
		idleAnim->AddTransition("Run", runningAnim);
		runningAnim->AddTransition("Idle", idleAnim);
	}

	// 캐릭터_핑크
	{
		Animation* idleAnim = ANIM("CP_Idle.anim");
		Animation* runningAnim = ANIM("CP_Run.anim");
		idleAnim->AddTransition("Run", runningAnim);
		runningAnim->AddTransition("Idle", idleAnim);
	}

	// 캐릭터_레드
	{
		Animation* idleAnim = ANIM("CR_Idle.anim");
		Animation* runningAnim = ANIM("CR_Run.anim");
		idleAnim->AddTransition("Run", runningAnim);
		runningAnim->AddTransition("Idle", idleAnim);
	}

	// NPC(세포)
	{
		Animation* idleAnim = ANIM("Cell_Idle.anim");
		Animation* runningAnim = ANIM("Cell_Run.anim");
		Animation* attackingAnim = ANIM("Cell_Attack.anim");

		idleAnim->AddTransition("Run", runningAnim);
		idleAnim->AddTransition("Attack", attackingAnim);
		runningAnim->AddTransition("Idle", idleAnim);
		runningAnim->AddTransition("Attack", attackingAnim);
	}

	// 탱크
	{
		Animation* idleAnim = ANIM("Tank_Idle.anim");
		Animation* runningAnim = ANIM("Tank_Run.anim");
		idleAnim->AddTransition("Run", runningAnim);
		runningAnim->AddTransition("Idle", idleAnim);
	}
}

void Client::processButton()
{
	if (Input::IsButtonPressed(eKeyCode::MouseLButton))
	{
		auto view = GetRegistry().view<ButtonComponent, RectTransformComponent>();
		for (auto [entity, button, rect] : view.each())
		{
			bool contains = Helpers::Intersects(rect.Position, rect.Width, rect.Height);

			if (contains)
			{
				button.CallbackFunc();
				break;
			}
		}
	}
}

void Client::updateMovement(float deltaTime)
{
	auto view = GetRegistry().view<MovementComponent, TransformComponent>();
	for (auto [entity, movement, transform] : view.each())
	{
		if (movement.Direction == Vector3::Zero)
		{
			continue;
		}

		Vector3 toward = transform.Position + movement.Direction * movement.MaxSpeed * deltaTime;
		Helpers::UpdatePosition(&transform.Position, toward, &transform.bDirty);

		// XMVector3AngleBetweenVectors() 로 구할 수 있는 각도는 0~180 까지다.
		// -x축으로 움직일 때, 제대로 된 각도로 회전하지 않으므로
		// -1 을 곱해 준다.
		Vector3 rotation = XMVector3AngleBetweenVectors(Vector3::UnitZ, movement.Direction);
		float scalar = 1.0f;
		if (movement.Direction.x < 0.0f)
		{
			scalar = -1.0f;
		}

		Helpers::UpdateYRotation(&transform.Rotation.y, scalar * XMConvertToDegrees(rotation.y), &transform.bDirty);
	}
}

void Client::updateScript(float deltaTime)
{
	auto view = GetRegistry().view<ScriptComponent>();
	for (auto [entity, script] : view.each())
	{
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
	for (auto [entity, animator] : view.each())
	{
		Helpers::UpdateAnimation(&animator, deltaTime);
	}
}

void Client::updateCollisionBox(float deltaTime)
{
	auto view = GetRegistry().view<BoxComponent, TransformComponent>();
	for (auto [entity, box, transform] : view.each())
	{
		Helpers::UpdateBox(box.Local, &box.World, transform.Position, transform.Rotation.y, transform.bDirty);
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
		Helpers::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);

		AnimatorComponent& animator = e.GetComponent<AnimatorComponent>();
		Helpers::BindBoneMatrix(animator.Palette, animator.Buffer);

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
			Helpers::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);
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
			Helpers::BindWorldMatrixAttached(&transform, &attach);
			MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();
			mRenderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
		}
	}
}

void Client::drawCollisionBox()
{
	gCmdList->SetPipelineState(mRenderer->GetWireframePSO().Get());
	auto view = GetRegistry().view<DebugDrawComponent, TransformComponent>();
	for (auto [entity, debugDraw, transform] : view.each())
	{
		Helpers::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);
		mRenderer->SubmitDebugMesh(debugDraw.Mesi);
	}
}

void Client::drawSpriteAndText()
{
	gCmdList->SetPipelineState(mRenderer->GetSpritePSO().Get());

	{
		auto view = GetRegistry().view<SpriteRendererComponent, RectTransformComponent>();
		for (auto [entity, sprite, rect] : view.each())
		{
			Helpers::BindWorldMatrix(rect.Position, &rect.Buffer, &rect.bDirty);
			mRenderer->SubmitSprite(sprite.Mesi, sprite.Tex);
		}
	}

	{
		auto view = GetRegistry().view<TextComponent, RectTransformComponent>();
		for (auto [entity, text, rect] : view.each())
		{
			Helpers::BindWorldMatrix(rect.Position, &rect.Buffer, &rect.bDirty);
			mRenderer->SubmitText(text.Txt);
		}
	}
}
