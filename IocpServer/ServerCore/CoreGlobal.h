#pragma once

extern shared_ptr<class IocpCore> gIocpCore;
extern shared_ptr<Concurrency::concurrent_queue<shared_ptr<class JobQueue>>> gJobQueue;
extern shared_ptr<class JobTimer> gJobTimer;