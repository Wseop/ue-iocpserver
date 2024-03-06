#include "pch.h"
#include "CoreTLS.h"
#include "JobQueue.h"

thread_local uint32 LThreadId = 0;
thread_local shared_ptr<JobQueue> LJobQueue = nullptr;