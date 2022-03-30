#pragma once

#include <xhash>

class HBID
{
public:
	HBID();
	HBID(uint64 id);

	operator uint64() { return mID; }
	operator const uint64() const { return mID; }

private:
	uint64 mID;
};

namespace std {

	template<>
	struct hash<HBID>
	{
		std::size_t operator()(const HBID& uuid) const
		{
			return hash<uint64>()((uint64)uuid);
		}
	};

}