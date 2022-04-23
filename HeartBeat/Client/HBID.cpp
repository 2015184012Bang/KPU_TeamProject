#include "ClientPCH.h"
#include "HBID.h"

#include <random>

static std::random_device sRandomDevice;
static std::mt19937_64 eng(sRandomDevice());
static std::uniform_int_distribution<uint64> sUID;

HBID::HBID()
	: mID(sUID(eng))
{

}

HBID::HBID(uint64 id)
	: mID(id)
{

}
