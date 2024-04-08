#pragma once

class Job;
class JobQueue;

struct ReservedJob
{
	ReservedJob(uint64 executeTick, shared_ptr<JobQueue> jobQueue, shared_ptr<Job>&& job) :
		executeTick(executeTick),
		jobQueue(jobQueue),
		job(job)
	{
	}

	bool operator<(const ReservedJob& other) const
	{
		return executeTick > other.executeTick;
	}

	uint64 executeTick = 0;
	shared_ptr<JobQueue> jobQueue = nullptr;
	shared_ptr<Job> job = nullptr;
};

class JobTimer
{
public:
	void ReserveJob(uint64 tickAfter, shared_ptr<JobQueue> jobQueue, shared_ptr<Job>&& job);
	void DistributeJobs(uint64 currentTick);

private:
	atomic<bool> _bDistributing = false;
	Concurrency::concurrent_priority_queue<shared_ptr<ReservedJob>> _reservedJobs;
};

