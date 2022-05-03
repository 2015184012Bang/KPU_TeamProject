#include "ClientPCH.h"
#include "Client.h"

#include "tinyxml2.h"

#include "Tags.h"
#include "Animation.h"
#include "Components.h"
#include "Define.h"
#include "GameMap.h"
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
#include "Random.h"

Client::Client()
{

}

Client::~Client()
{

}

bool Client::Init()
{
	Input::Init();
	Timer::Init();
	SoundManager::Init();
	Random::Init();
	Values::Init();

	gGameMap.LoadMap("../Assets/Maps/Map01.csv");

	mPacketManager = std::make_unique<PacketManager>();
	mPacketManager->Init();

	mRenderer = std::make_unique<Renderer>();
	mRenderer->Init();

	ResourceManager::MakeAnimTransitions();

	createCameraEntity();

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
	Entity e = Entity{ gRegistry.create() };

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
	Entity e = Entity{ gRegistry.create() };

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
	Entity e = Entity{ gRegistry.create() };

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
	Entity e = Entity{ gRegistry.create() };

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
	Entity e = Entity{ gRegistry.create() };

	e.AddTag<Tag_Sprite>();
	e.AddComponent<RectTransformComponent>(width, height);
	e.AddComponent<SpriteRendererComponent>(new SpriteMesh(width, height), texFile, drawOrder);

	// Do sorting by draw order when sprite is added
	gRegistry.sort<SpriteRendererComponent>([](const auto& lhs, const auto& rhs)
		{
			return lhs.DrawOrder < rhs.DrawOrder;
		});

	return e;
}

Entity Client::CreateTextEntity(const Font* fontFile)
{
	Entity e = Entity{ gRegistry.create() };

	e.AddTag<Tag_Text>();
	e.AddComponent<RectTransformComponent>(0, 0);
	e.AddComponent<TextComponent>(new Text(fontFile));

	return e;
}

void Client::DestroyEntityAfter(const uint32 eid, float secs)
{
	mPendingEntities.emplace_back(eid, secs);
}

void Client::RearrangeAttachment()
{
	auto view = gRegistry.view<AttachmentParentComponent, TransformComponent, AnimatorComponent>();

	for (auto [entity, parent, transform, animator] : view.each())
	{
		for (auto [boneName, eid] : parent.Children)
		{
			Entity child = Entity{ eid };
			auto& attachment = child.GetComponent<AttachmentChildComponent>();

			attachment.BoneIndex = animator.Skel->GetBoneIndexByName(boneName);
			attachment.ParentPalette = &animator.Palette;
			attachment.ParentTransform = &transform;
		}
	}
}


void Client::SetFollowCameraTarget(const Entity& target, const Vector3& offset)
{
	mFollowCameraTarget = target;
	mTargetOffset = offset;
}

void Client::processInput()
{
	Input::Update();

	if (Input::IsButtonPressed(KeyCode::ESCAPE))
	{
		mbRunning = false;
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

	processPendingEntities(deltaTime);
	updateMovement(deltaTime);
	updateScript(deltaTime);
	updateAnimation(deltaTime);
	updateCollisionBox(deltaTime);
	
	if (mFollowCameraTarget)
	{
		updateMainCamera();
	}

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
	mMainCamera = Entity{ gRegistry.create() };
	mMainCamera.AddComponent<CameraComponent>(Vector3(0.0f, 500.0f, -500.0f), Vector3(0.0f, 0.0f, 0.0f));
	mMainCamera.AddTag<Tag_Camera>();
	mMainCamera.AddTag<Tag_DontDestroyOnLoad>();

	m2dCamera = Entity{ gRegistry.create() };
	m2dCamera.AddComponent<CameraComponent>();
	m2dCamera.AddTag<Tag_Camera>();
	m2dCamera.AddTag<Tag_DontDestroyOnLoad>();
}

void Client::processButton()
{
	if (Input::IsButtonPressed(KeyCode::MOUSE_L))
	{
		auto view = gRegistry.view<ButtonComponent, RectTransformComponent>();
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

void Client::processPendingEntities(float deltaTime)
{
	auto iter = mPendingEntities.begin();
	while (iter != mPendingEntities.end())
	{
		iter->second -= deltaTime;

		if (iter->second < 0.0f)
		{
			DestroyEntityByID(iter->first);
			iter = mPendingEntities.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void Client::updateMovement(float deltaTime)
{
	auto view = gRegistry.view<MovementComponent, TransformComponent>();
	for (auto [entity, movement, transform] : view.each())
	{
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

		float yaw = XMConvertToDegrees(rotation.y);

		if (isnan(yaw))
		{
			continue;
		}

		Helpers::UpdateYRotation(&transform.Rotation.y, scalar * XMConvertToDegrees(rotation.y), &transform.bDirty);
	}
}

void Client::updateScript(float deltaTime)
{
	auto view = gRegistry.view<ScriptComponent>();
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
	auto view = gRegistry.view<AnimatorComponent>();
	for (auto [entity, animator] : view.each())
	{
		Helpers::UpdateAnimation(&animator, deltaTime);
	}
}

void Client::updateCollisionBox(float deltaTime)
{
	auto view = gRegistry.view<BoxComponent, TransformComponent>();
	for (auto [entity, box, transform] : view.each())
	{
		Helpers::UpdateBox(box.Local, &box.World, transform.Position, transform.Rotation.y, transform.bDirty);
	}
}

void Client::updateMainCamera()
{
	const auto& targetPosition = mFollowCameraTarget.GetComponent<TransformComponent>().Position;
	auto& cc = mMainCamera.GetComponent<CameraComponent>();
	cc.Target = targetPosition;
	cc.Position = { targetPosition.x + mTargetOffset.x, mTargetOffset.y, targetPosition.z + mTargetOffset.z };
}

void Client::drawSkeletalMesh()
{
	gCmdList->SetPipelineState(mRenderer->GetSkeletalMeshPSO().Get());
	auto view = gRegistry.view<Tag_SkeletalMesh>();
	for (auto entity : view)
	{
		Entity e = Entity{ entity };

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
		auto view = gRegistry.view<Tag_StaticMesh>(entt::exclude<AttachmentChildComponent>);
		for (auto entity : view)
		{
			Entity e = Entity{ entity };

			TransformComponent& transform = e.GetComponent<TransformComponent>();
			Helpers::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);
			MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();
			mRenderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
		}
	}

	{
		auto view = gRegistry.view<AttachmentChildComponent>();
		for (auto entity : view)
		{
			Entity e = Entity{ entity };

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
	auto view = gRegistry.view<DebugDrawComponent, TransformComponent>();
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
		auto view = gRegistry.view<SpriteRendererComponent, RectTransformComponent>();
		for (auto [entity, sprite, rect] : view.each())
		{
			Helpers::BindWorldMatrix(rect.Position, &rect.Buffer, &rect.bDirty);
			mRenderer->SubmitSprite(sprite.Mesi, sprite.Tex);
		}
	}

	{
		auto view = gRegistry.view<TextComponent, RectTransformComponent>();
		for (auto [entity, text, rect] : view.each())
		{
			Helpers::BindWorldMatrix(rect.Position, &rect.Buffer, &rect.bDirty);
			mRenderer->SubmitText(text.Txt);
		}
	}
}
