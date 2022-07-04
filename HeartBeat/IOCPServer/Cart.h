#pragma once

#include "pch.h"
#include "Script.h"

#include "Timer.h"

class Cart : public Script
{
public:
	Cart(entt::registry& registry, entt::entity owner)
		: Script{ registry, owner } {}

	virtual void Start() override
	{
		// 최종 타일의 위치를 구한다.
		auto endTile = GetEntityByName(mRegistry, "EndPoint");
		ASSERT(mRegistry.valid(endTile), "Invalid entity!");
		mEndPoint = mRegistry.get<TransformComponent>(endTile).Position;
		mEndPoint.y = 0.0f;

		// 시작 타일의 위치를 구한다.
		auto startTile = GetEntityByName(mRegistry, "StartPoint");
		ASSERT(mRegistry.valid(startTile), "Invalid entity!");
		const auto& startTilePosition = mRegistry.get<TransformComponent>(startTile).Position;

		// 시작 타일의 위치에서 -x축으로 좀 떨어진 곳을 카트의 시작 위치로 한다.
		auto& transform = GetComponent<TransformComponent>();
		transform.Position.x = startTilePosition.x - 800.0f;
		transform.Position.z = startTilePosition.z;

		// 현재 타겟을 시작 타일의 위치로 한다.
		// RAIL_TILE은 y 위치가 -TILE_SIDE 만큼이므로
		// y값은 0으로 해준다.
		mCurrentTarget = startTilePosition;
		mCurrentTarget.y = 0.0f;

		// 초기 방향 설정
		Vector3 direction = Vector3::Zero;
		direction = XMVectorSubtract(mCurrentTarget, transform.Position);
		direction.Normalize();
		auto& movement = GetComponent<MovementComponent>();
		movement.Direction = direction;

		auto view = mRegistry.view<Tag_RailTile, TransformComponent>();
		for (auto [entity, tileTransform] : view.each())
		{
			if (entity == startTile ||
				startTilePosition.x > tileTransform.Position.x)
			{
				continue;
			}

			mTiles.push_back(tileTransform.Position);
		}
	}

	virtual void Update() override
	{
		if (HasComponent<Tag_Stop>())
		{
			return;
		}

		const auto& position = GetComponent<TransformComponent>().Position;
		const float posToEnd = Vector3::DistanceSquared(position, mEndPoint);

		// 최종 목표 지점과 충분히 가까워졌다면,
		// 이동 방향을 0로 만든다.
		if (posToEnd < CLOSE_ENOUGH)
		{
			GetComponent<MovementComponent>().Direction = Vector3::Zero;
			return;
		}

		// 위치-현재 목표의 거리가 CLOSE_ENOUGH보다 작다면,
		// 그 다음 목표를 구하고 이동 방향을 설정한다.
		const float posToCur = Vector3::DistanceSquared(position, mCurrentTarget);
		if (posToCur < CLOSE_ENOUGH)
		{
			float temp = FLT_MAX;
			INT32 minIndex = -1;
			for (int i = 0; i < mTiles.size(); ++i)
			{
				float dist = Vector3::DistanceSquared(position, mTiles[i]);
				if (temp > dist)
				{
					minIndex = i;
					temp = dist;
				}
			}

			mCurrentTarget = mTiles[minIndex];
			mCurrentTarget.y = 0.0f;

			Vector3 direction = XMVectorSubtract(mCurrentTarget, position);
			direction.Normalize();
			GetComponent<MovementComponent>().Direction = direction;

			iter_swap(mTiles.begin() + minIndex, mTiles.end() - 1);
			mTiles.pop_back();
		}
	}

private:
	vector<Vector3> mTiles;
	Vector3 mCurrentTarget = Vector3::Zero;
	Vector3 mEndPoint = Vector3::Zero;

	bool mbArrive = false;

	const float CLOSE_ENOUGH = 10.0f * 10.0f;
};