#include "ClientPCH.h"
#include "Client.h"

#include "tinyxml2.h"

#include "Tags.h"
#include "Animation.h"
#include "Components.h"
#include "GameMap.h"
#include "Helpers.h"
#include "Input.h"
#include "LoginScene.h"
#include "Mesh.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "GameScene.h"
#include "PacketManager.h"
#include "Timer.h"
#include "Utils.h"
#include "Script.h"
#include "Mesh.h"
#include "Texture.h"
#include "Skeleton.h"
#include "SoundManager.h"
#include "TestScene.h"
#include "Random.h"

bool gShouldClose = false;

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

	// Parent 엔티티가 삭제될 때 Child도 삭제하도록 한다.
	gRegistry.on_destroy<ParentComponent>().connect<&Client::DeleteChildren>(this);

	gGameMap.LoadMap("../Assets/Maps/Map.csv");

	mPacketManager = std::make_unique<PacketManager>();
	mPacketManager->Init();

	mRenderer = std::make_unique<Renderer>();
	mRenderer->Init();

	ResourceManager::MakeAnimTransitions();

	createCameraEntity();
	createLightEntity();

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

void Client::DestroyEntityAfter(const uint32 eid, float secs)
{
	mPendingEntities.emplace_back(eid, secs);
}

void Client::SetFollowCameraTarget(const Entity& target, const Vector3& offset)
{
	mFollowCameraTarget = target;
	mTargetOffset = offset;
}

void Client::DisableFollowTarget()
{
	mFollowCameraTarget = {};
}

void Client::ResetCamera()
{
	mFollowCameraTarget = {};
	auto& cc = mMainCamera.GetComponent<CameraComponent>();
	cc.Position = Vector3{ 0.0f, 500.0f, -500.0f };
	cc.Target = Vector3::Zero;
}

void Client::DeleteChildren(entt::registry& regi, entt::entity entity)
{
	Entity parent = Entity{ entity };
	auto& children = parent.GetComponent<ParentComponent>().Children;

	for (auto eid : children)
	{
		// DestroyAll()을 수행하면 부모보다 자식이 먼저 삭제될 수 있다.
		// 유효성 검사 필수.
		bool bValid = regi.valid(eid);
		if (!bValid)
		{
			continue;
		}

		regi.destroy(eid);
	}
}

void Client::SetBackgroundColor(const XMVECTORF32& color)
{
	mRenderer->SetBackgroundColor(color);
}

void Client::processInput()
{
	Input::Update();

#ifdef _DEBUG
	if (Input::IsButtonPressed(KeyCode::ESCAPE))
	{
		gShouldClose = true;
	}
#endif

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
	updateMainCamera();

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

	// 조명 설정
	auto& light = mLight.GetComponent<LightComponent>();
	light.Light.CameraPosition = camera.Position;
	Helpers::BindLight(&light);

	drawSkeletalMesh();
	drawStaticMesh();

#ifdef _DEBUG
	drawCollisionBox();
#endif

	// Set 2D camera
	auto& spriteCamera = m2dCamera.GetComponent<CameraComponent>();
	Helpers::BindViewProjectionMatrixOrtho(spriteCamera.Buffer);

	drawSprite();

	mActiveScene->Render(mRenderer);

	mRenderer->EndRender();

	// D2D를 이용한 폰트 렌더링
	drawFont();

	mRenderer->Present();
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

void Client::createLightEntity()
{
	mLight = Entity{ gRegistry.create() };
	mLight.AddTag<Tag_DontDestroyOnLoad>();
	auto& light = mLight.AddComponent<LightComponent>();
	light.Light.AmbientColor = Vector3{ 0.35f, 0.35f, 0.35f };
	light.Light.LightPosition = Vector3{ 10000.0f, 50000.f, 2000.0f };
	light.Light.SpecularStrength = 0.0f;
	light.Light.CameraPosition = mMainCamera.GetComponent<CameraComponent>().Position;
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

	{
		auto view = gRegistry.view<FollowComponent, TransformComponent>();
		for (auto [entity, follow, transform] : view.each())
		{
			auto target = GetEntityByID(follow.TargetID);
			if (!gRegistry.valid(target))
			{
				continue;
			}

			const auto& targetPos = target.GetComponent<TransformComponent>().Position;

			Helpers::UpdatePosition(&transform.Position,
				Vector3{ targetPos.x, transform.Position.y, targetPos.z },
				&transform.bDirty);
		}
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

void Client::updateCameraShake()
{
	auto view = gRegistry.view<CameraComponent, CameraShakeComponent>();

	for (auto [entity, cc, cs] : view.each())
	{
		if (cs.bShakeX)
		{
			auto offset = Random::RandInt(-200, 200);
			cc.Position = cs.OrigCameraPos;
			cc.Position.x += offset;
			cc.Target = cs.OrigCameraTarget;
			cc.Target.x += offset;
		}

		else
		{
			auto offset = Random::RandInt(-200, 200);
			cc.Position = cs.OrigCameraPos;
			cc.Position.z += offset;
			cc.Target = cs.OrigCameraTarget;
			cc.Target.z += offset;
		}
	}
}

void Client::updateMainCamera()
{
	if (mMainCamera.HasComponent<CameraShakeComponent>())
	{
		updateCameraShake();
	}

	else if (mFollowCameraTarget)
	{
		const auto& targetPosition = mFollowCameraTarget.GetComponent<TransformComponent>().Position;
		auto& cc = mMainCamera.GetComponent<CameraComponent>();
		cc.Target = targetPosition;
		cc.Position = { targetPosition.x + mTargetOffset.x, mTargetOffset.y, targetPosition.z + mTargetOffset.z };
	}
}

void Client::drawSkeletalMesh()
{
	gCmdList->SetPipelineState(mRenderer->GetSkeletalMeshPSO().Get());

	{
		auto view = gRegistry.view<Tag_SkeletalMesh>(entt::exclude<ChildComponent>);
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

	{
		auto view = gRegistry.view<Tag_SkeletalMesh, ChildComponent>();
		for (auto [entity, child] : view.each())
		{
			Entity e = Entity{ entity };

			TransformComponent& transform = e.GetComponent<TransformComponent>();
			Helpers::BindWorldMatrixAttached(&transform, &child);

			AnimatorComponent& animator = e.GetComponent<AnimatorComponent>();
			Helpers::BindBoneMatrix(animator.Palette, animator.Buffer);

			MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();
			mRenderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
		}
	}
}

void Client::drawStaticMesh()
{
	// 배경 셰이더
	gCmdList->SetPipelineState(mRenderer->GetBackgroundPSO().Get());
	{
		auto view = gRegistry.view<Tag_Background>();
		for (auto entity : view)
		{
			Entity e = Entity{ entity };

			TransformComponent& transform = e.GetComponent<TransformComponent>();
			Helpers::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);
			MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();
			mRenderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
		}
	}

	// 조명 미적용 셰이더
	gCmdList->SetPipelineState(mRenderer->GetNoLightPSO().Get());
	{
		auto view = gRegistry.view<Tag_StaticMesh, Tag_Tile>();
		for (auto entity : view)
		{
			Entity e = Entity{ entity };

			TransformComponent& transform = e.GetComponent<TransformComponent>();
			Helpers::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);
			MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();
			mRenderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
		}
	}

	// 조명 적용 셰이더
	gCmdList->SetPipelineState(mRenderer->GetStaticMeshPSO().Get());
	{
		auto view = gRegistry.view<Tag_StaticMesh>(entt::exclude<Tag_Tile>);
		for (auto entity : view)
		{
			Entity e = Entity{ entity };

			TransformComponent& transform = e.GetComponent<TransformComponent>();

			if (e.HasComponent<ChildComponent>())
			{
				ChildComponent& attach = e.GetComponent<ChildComponent>();
				Helpers::BindWorldMatrixAttached(&transform, &attach);
			}
			else
			{
				Helpers::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, &transform.Buffer, &transform.bDirty);
			}

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

void Client::drawSprite()
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
}

void Client::drawFont()
{
	auto view = gRegistry.view<TextComponent>();

	if (view.empty())
	{
		return;
	}

	vector<Sentence> sentences;
	sentences.reserve(view.size());
	for (auto [entity, text] : view.each())
	{
		sentences.emplace_back(&text.Sentence, (UINT32)text.Sentence.size(), text.X,
			text.Y, text.FontSize);
	}

	mRenderer->RenderFont(sentences);
}
