#pragma once

class JobQueue;
class Job;

class JobTimerItem
{
public:
	JobTimerItem(uint64 executeTick, weak_ptr<JobQueue> jobOwner, shared_ptr<Job> job);

	bool operator<(const JobTimerItem& other) const;

public:
	uint64 GetExecuteTick() const { return _executeTick; }
	shared_ptr<JobQueue> GetJobOwner() const { return _jobOwner.lock(); }
	shared_ptr<Job> GetJob() const { return _job; }

private:
	uint64 _executeTick = 0;
	weak_ptr<JobQueue> _jobOwner;
	shared_ptr<Job> _job;
};

class JobTimer
{
public:
	void Reserve(uint64 executeTick, weak_ptr<JobQueue> jobOwner, shared_ptr<Job> job);
	void Distribute(uint64 currentTick);

private:
	Concurrency::concurrent_priority_queue<shared_ptr<JobTimerItem>> _items;
	atomic<bool> _bDistributing = false;
};

