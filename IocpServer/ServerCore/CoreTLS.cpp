#include "pch.h"
#include "CoreTLS.h"
#include "JobQueue.h"

thread_local uint32 tThreadId = 0;
thread_local shared_ptr<JobQueue> tJobQueue = nullptr;