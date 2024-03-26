#pragma once

class Job;

class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	virtual ~JobQueue();

public:
	void Push(shared_ptr<Job> job, bool pushOnly = false);
	void Execute();

private:
	LockQueue<shared_ptr<Job>> _jobs;
	atomic<uint32> _jobCount = 0;
};

