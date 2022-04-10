#pragma once

#include <vector>

#include "HeartBeat/Script.h"
#include "HeartBeat/GameMap.h"
#include "HeartBeat/Log.h"
#include "HeartBeat/Tags.h"

#include "ServerSystems.h"

class Tank : public Script
{
public:
	Tank(Entity owner)
		: Script(owner)
	{}

	virtual void Start() override
	{
		transform = &GetComponent<STransformComponent>();

		for (const auto& tile : gGameMap.GetTiles())
		{
			if (tile.Type == Rail)
			{
				mPath.push(tile);
			}
		}
		
		bool retVal = GetNextTarget(&mCurrentTarget);
		HB_ASSERT((retVal), "Failed to get next target");
	}

	virtual void Update(float deltaTime) override
	{
		if (mbArrive)
		{
			return;
		}

		Vector3 to = Vector3(mCurrentTarget.X, 0.0f, mCurrentTarget.Z);

		ServerSystems::Move(&transform->Position, to, Timer::GetDeltaTime());
		AddTag<Tag_UpdateTransform>();

		if (NearZero(transform->Position, to))
		{
			bool retVal = GetNextTarget(&mCurrentTarget);

			if (!retVal)
			{
				mbArrive = true;
			}
		}
	}

	bool GetNextTarget(Tile* outTarget)
	{
		if (mPath.empty())
		{
			return false;
		}

		*outTarget = mPath.front();
		mPath.pop();

		return true;
	}

private:
	std::queue<Tile> mPath;
	Tile mCurrentTarget;

	STransformComponent* transform = nullptr;

	bool mbArrive = false;
};
