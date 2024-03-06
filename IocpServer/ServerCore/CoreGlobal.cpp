#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "JobQueue.h"

shared_ptr<ThreadManager> GThreadManager = nullptr;
shared_ptr<LockQueue<shared_ptr<JobQueue>>> GJobQueue = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = make_shared<ThreadManager>();
		GJobQueue = make_shared<LockQueue<shared_ptr<JobQueue>>>();

		SocketUtils::Init();
	}

	~CoreGlobal()
	{
		SocketUtils::Clear();
	}
}GCoreGlobal;