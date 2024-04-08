#include "pch.h"
#include "CoreTLS.h"

thread_local shared_ptr<JobQueue> tJobQueue = nullptr;