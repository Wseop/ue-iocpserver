#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "JobQueue.h"

shared_ptr<ThreadManager> gThreadManager = nullptr;
shared_ptr<LockQueue<shared_ptr<JobQueue>>> gJobQueue = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		gThreadManager = make_shared<ThreadManager>();
		gJobQueue = make_shared<LockQueue<shared_ptr<JobQueue>>>();

		SocketUtils::Init();
	}

	~CoreGlobal()
	{
		SocketUtils::Clear();
	}
} gCoreGlobal;