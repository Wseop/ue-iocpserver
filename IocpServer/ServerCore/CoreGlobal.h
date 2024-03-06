#pragma once

extern shared_ptr<class ThreadManager> GThreadManager;
extern shared_ptr<LockQueue<shared_ptr<class JobQueue>>> GJobQueue;