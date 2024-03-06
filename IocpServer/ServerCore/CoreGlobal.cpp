#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"

shared_ptr<ThreadManager> GThreadManager = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = make_shared<ThreadManager>();

		SocketUtils::Init();
	}

	~CoreGlobal()
	{
		SocketUtils::Clear();
	}
}GCoreGlobal;