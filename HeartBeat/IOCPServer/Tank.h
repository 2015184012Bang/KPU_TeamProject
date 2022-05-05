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
		// ���� Ÿ���� ��ġ�� ���Ѵ�.
		Entity endTile = GetEntityByName("EndPoint");
		ASSERT(endTile, "Invalid entity!");
		mEndPoint = endTile.GetComponent<TransformComponent>().Position;
		mEndPoint.y = 0.0f;

		// ���� Ÿ���� ��ġ�� ���Ѵ�.
		Entity startTile = GetEntityByName("StartPoint");
		ASSERT(startTile, "Invalid entity!");
		const auto& tilePosition = startTile.GetComponent<TransformComponent>().Position;

		// ���� Ÿ���� ��ġ�� ��ũ�� ���� ��ġ�� �Ѵ�.
		auto& transform = GetComponent<TransformComponent>();
		transform.Position.x = tilePosition.x;
		transform.Position.z = tilePosition.z;

		// ���� Ÿ���� ���� Ÿ���� ��ġ�� �Ѵ�.
		// RAIL_TILE�� y ��ġ�� -TILE_SILE ��ŭ�̹Ƿ�
		// y���� 0���� ���ش�.
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