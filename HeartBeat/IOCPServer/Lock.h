#pragma once

class Lock
{
public:
	Lock();

	void ReadLock();

	void ReadUnLock();

	void WriteLock();

	void WriteUnlock();

private:
	SRWLOCK mLock;
};

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock)
		: mLock(lock)
	{
		mLock.ReadLock();
	}

	~ReadLockGuard()
	{
		mLock.ReadUnLock();
	}

private:
	Lock& mLock;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock)
		: mLock(lock)
	{
		mLock.WriteLock();
	}

	~WriteLockGuard()
	{
		mLock.WriteUnlock();
	}

private:
	Lock& mLock;
};