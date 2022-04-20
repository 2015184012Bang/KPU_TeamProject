#include "pch.h"
#include "Lock.h"

Lock::Lock()
{
	InitializeSRWLock(&mLock);
}

void Lock::ReadLock()
{
	AcquireSRWLockShared(&mLock);
}

void Lock::ReadUnLock()
{
	ReleaseSRWLockShared(&mLock);
}

void Lock::WriteLock()
{
	AcquireSRWLockExclusive(&mLock);
}

void Lock::WriteUnlock()
{
	ReleaseSRWLockExclusive(&mLock);
}
