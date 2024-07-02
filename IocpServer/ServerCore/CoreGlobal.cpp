#include "pch.h"
#include "CoreGlobal.h"
#include "JobQueue.h"
#include "JobTimer.h"

shared_ptr<Concurrency::concurrent_queue<shared_ptr<JobQueue>>> gJobQueue = nullptr;
shared_ptr<JobTimer> gJobTimer = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		SocketUtils::startUp();

		gJobQueue = make_shared<Concurrency::concurrent_queue<shared_ptr<JobQueue>>>();
		gJobTimer = make_shared<JobTimer>();
	}

	~CoreGlobal()
	{
		SocketUtils::cleanUp();
	}
} gCoreGlobal;