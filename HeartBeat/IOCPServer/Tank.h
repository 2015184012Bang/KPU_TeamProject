#pragma once

#include "pch.h"
#include "Script.h"

#include "Timer.h"

#include "ServerSystems.h"

class Tank : public Script
{
public:
	Tank(Entity owner)
		: Script(owner) {}

	virtual void Start() override
	{
		// 최종 타일의 위치를 구한다.
		Entity endTile = GetEntityByName("EndPoint");
		ASSERT(endTile, "Invalid entity!");
		mEndPoint = endTile.GetComponent<TransformComponent>().Position;
		mEndPoint.y = 0.0f;

		// 시작 타일의 위치를 구한다.
		Entity startTile = GetEntityByName("StartPoint");
		ASSERT(startTile, "Invalid entity!");
		const auto& tilePosition = startTile.GetComponent<TransformComponent>().Position;

		// 시작 타일의 위치를 탱크의 시작 위치로 한다.
		auto& transform = GetComponent<TransformComponent>();
		transform.Position.x = tilePosition.x;
		transform.Position.z = tilePosition.z;

		// 현재 타겟을 시작 타일의 위치로 한다.
		// RAIL_TILE은 y 위치가 -TILE_SILE 만큼이므로
		// y값은 0으로 해준다.
		mCurrentTarget = tilePosition;
		mCurrentTarget.y = 0.0f;
	
		auto view = gRegistry.view<Tag_RailTile, TransformComponent>();
		for (auto [entity, tileTransform] : view.each())
		{
			mTiles.push_back(tileTransform.Position);
		}
	}

	virtual void Update() override
	{
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