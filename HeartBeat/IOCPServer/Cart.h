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
		// ���� Ÿ���� ��ġ�� ���Ѵ�.
		auto endTile = GetEntityByName(mRegistry, "EndPoint");
		ASSERT(mRegistry.valid(endTile), "Invalid entity!");
		mEndPoint = mRegistry.get<TransformComponent>(endTile).Position;
		mEndPoint.y = 0.0f;

		// ���� Ÿ���� ��ġ�� ���Ѵ�.
		auto startTile = GetEntityByName(mRegistry, "StartPoint");
		ASSERT(mRegistry.valid(startTile), "Invalid entity!");
		const auto& startTilePosition = mRegistry.get<TransformComponent>(startTile).Position;

		// ���� Ÿ���� ��ġ���� -x������ �� ������ ���� īƮ�� ���� ��ġ�� �Ѵ�.
		auto& transform = GetComponent<TransformComponent>();
		transform.Position.x = startTilePosition.x - 800.0f;
		transform.Position.z = startTilePosition.z;

		// ���� Ÿ���� ���� Ÿ���� ��ġ�� �Ѵ�.
		// RAIL_TILE�� y ��ġ�� -TILE_SIDE ��ŭ�̹Ƿ�
		// y���� 0���� ���ش�.
		mCurrentTarget = startTilePosition;
		mCurrentTarget.y = 0.0f;

		// �ʱ� ���� ����
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

		// ���� ��ǥ ������ ����� ��������ٸ�,
		// �̵� ������ 0�� �����.
		if (posToEnd < CLOSE_ENOUGH)
		{
			GetComponent<MovementComponent>().Direction = Vector3::Zero;
			return;
		}

		// ��ġ-���� ��ǥ�� �Ÿ��� CLOSE_ENOUGH���� �۴ٸ�,
		// �� ���� ��ǥ�� ���ϰ� �̵� ������ �����Ѵ�.
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