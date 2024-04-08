#include "pch.h"
#include "JobTimer.h"
#include "Job.h"
#include "JobQueue.h"

void JobTimer::ReserveJob(uint64 tickAfter, shared_ptr<JobQueue> jobQueue, shared_ptr<Job>&& job)
{
	uint64 executeTick = ::GetTickCount64() + tickAfter;
	_reservedJobs.push(make_shared<ReservedJob>(executeTick, jobQueue, move(job)));
}

void JobTimer::DistributeJobs(uint64 currentTick)
{
	if (_bDistributing.exchange(true) == true)
		return;

	shared_ptr<ReservedJob> reservedJob = nullptr;
	while (_reservedJobs.try_pop(reservedJob))
	{
		// 아직 실행할 시간이 아니면 다시 push 후 distribute 종료
		if (reservedJob->executeTick > currentTick)
		{
			_reservedJobs.push(reservedJob);
			break;
		}

		reservedJob->jobQueue->Push(move(reservedJob->job));
	}

	_bDistributing.store(false);
}
