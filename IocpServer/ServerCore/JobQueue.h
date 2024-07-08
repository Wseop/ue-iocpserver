#pragma once

class Job;

class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	virtual ~JobQueue();

public:
	void push(shared_ptr<Job>&& job);
	void execute();

private:
	mutex _mutex;
	bool _bExecuting = false;
	queue<shared_ptr<Job>> _jobs;
};

