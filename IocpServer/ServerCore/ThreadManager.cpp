#include "pch.h"
#include "ThreadManager.h"
#include "JobQueue.h"
#include "JobTimer.h"

ThreadManager::ThreadManager()
{
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void(void)> callback)
{
	lock_guard<mutex> lock(_mutex);

	_threads.push_back(thread([callback]()
		{
			InitTLS();
			callback();
		}));
}

void ThreadManager::Join()
{
	for (thread& t : _threads)
	{
		if (t.joinable())
			t.join();
	}
	_threads.clear();
}

void ThreadManager::InitTLS()
{
	static atomic<uint32> sThreadId = 0;
	tThreadId = sThreadId++;
}

void ThreadManager::ExecuteJobQueue()
{
	// ����ϰ� �ִ� JobQueue�� ���ٸ�, GlobalQueue�� �ִ� JobQueue�� �����ͼ� ó��
	if (tJobQueue == nullptr && gJobQueue->try_pop(tJobQueue))
	{
		tJobQueue->Execute();
	}
}

void ThreadManager::DistributeReservedJob()
{
	gJobTimer->Distribute(::GetTickCount64());
}
