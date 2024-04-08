#pragma once

class Job;

class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	virtual ~JobQueue();

public:
	void Push(shared_ptr<Job>&& job, bool bPushOnly = false);
	void Execute();

private:
	atomic<uint32> _jobCount = 0;
	Concurrency::concurrent_queue<shared_ptr<Job>> _jobs;
};

