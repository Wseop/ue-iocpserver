#include "pch.h"
#include "ThreadManager.h"
#include "JobQueue.h"

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
	static atomic<uint32> SThreadId = 0;
	LThreadId = SThreadId++;
}

void ThreadManager::ExecuteJobQueue()
{
	// ����ϰ� �ִ� JobQueue�� ���ٸ�, GlobalQueue�� �ִ� JobQueue�� �����ͼ� ó��
	if (LJobQueue == nullptr && GJobQueue->Pop(LJobQueue))
	{
		LJobQueue->Execute();
	}
}
