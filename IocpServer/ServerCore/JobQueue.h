#pragma once

class Job;

class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	virtual ~JobQueue();

public:
	void Push(shared_ptr<Job> job);
	void Execute();

private:
	Concurrency::concurrent_queue<shared_ptr<Job>> _jobs;
	atomic<uint32> _jobCount = 0;
};

