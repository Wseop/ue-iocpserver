#pragma once

extern shared_ptr<class ThreadManager> gThreadManager;
extern shared_ptr<LockQueue<shared_ptr<class JobQueue>>> gJobQueue;
extern shared_ptr<class JobTimer> gJobTimer;