#include "pch.h"
#include "JobTimer.h"
#include "JobQueue.h"

JobTimerItem::JobTimerItem(uint64 executeTick, weak_ptr<JobQueue> jobOwner, shared_ptr<Job> job) :
	_executeTick(executeTick),
	_jobOwner(jobOwner),
	_job(job)
{
}

bool JobTimerItem::operator<(const JobTimerItem& other) const
{
	return _executeTick > other._executeTick;
}

void JobTimer::Reserve(uint64 executeTick, weak_ptr<JobQueue> jobOwner, shared_ptr<Job> job)
{
	uint64 tick = ::GetTickCount64() + executeTick;
	shared_ptr<JobTimerItem> item = make_shared<JobTimerItem>(tick, jobOwner, job);
	_items.push(item);
}

void JobTimer::Distribute(uint64 currentTick)
{
	if (_bDistributing.exchange(true) == true)
		return;

	shared_ptr<JobTimerItem> item = nullptr;
	while (_items.try_pop(item))
	{
		if (item->GetExecuteTick() > currentTick)
		{
			_items.push(item);
			break;
		}

		if (shared_ptr<JobQueue> jobOwner = item->GetJobOwner())
			jobOwner->Push(item->GetJob());
	}

	_bDistributing.store(false);
}
