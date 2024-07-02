#include "pch.h"
#include "ThreadManager.h"
#include "JobQueue.h"
#include "JobTimer.h"

ThreadManager* ThreadManager::_instance = nullptr;

ThreadManager::ThreadManager()
{
}

ThreadManager::~ThreadManager()
{
	join();
}

ThreadManager* ThreadManager::instance()
{
	if (_instance == nullptr)
	{
		_instance = new ThreadManager();
	}

	return _instance;
}

void ThreadManager::launch(function<void(void)> callback)
{
	lock_guard<mutex> lock(_mutex);
	
	_threads.push_back(thread(callback));
}

void ThreadManager::join()
{
	for (thread& t : _threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
	_threads.clear();
}

void ThreadManager::executeJobQueue()
{
	if (tJobQueue == nullptr && gJobQueue->try_pop(tJobQueue))
	{
		tJobQueue->Execute();
	}
}

void ThreadManager::distributeReservedJob()
{
	gJobTimer->DistributeJobs(::GetTickCount64());
}
