#include "pch.h"
#include "ThreadManager.h"
#include "JobQueue.h"
#include "JobTimer.h"

ThreadManager::ThreadManager()
{
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void(void)> callback)
{
	lock_guard<mutex> lock(_mutex);

	_threads.push_back(thread([this, callback]()
		{
			while (true)
			{
				callback();
				ExecuteJob();
				DistributeReservedJob();
			}
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

void ThreadManager::ExecuteJob()
{
	if (tJobQueue == nullptr && gJobQueue->try_pop(tJobQueue))
		tJobQueue->Execute();
}

void ThreadManager::DistributeReservedJob()
{
	gJobTimer->DistributeJobs(::GetTickCount64());
}
