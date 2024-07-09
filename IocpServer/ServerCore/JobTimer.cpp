#include "pch.h"
#include "JobTimer.h"
#include "Job.h"
#include "JobQueue.h"

void JobTimer::reserveJob(uint64 tickAfter, shared_ptr<JobQueue> jobQueue, shared_ptr<Job>&& job)
{
	uint64 executeTick = ::GetTickCount64() + tickAfter;
	_reservedJobs.push(make_shared<ReservedJob>(executeTick, jobQueue, move(job)));
}

void JobTimer::distributeJobs(uint64 currentTick)
{
	if (_bDistributing.exchange(true) == true)
		return;

	while (_reservedJobs.empty() == false)
	{
		shared_ptr<ReservedJob> reservedJob = _reservedJobs.top();
		_reservedJobs.pop();

		// ���� ������ �ð��� �ƴϸ� �ٽ� push �� distribute ����
		if (reservedJob->executeTick > currentTick)
		{
			_reservedJobs.push(reservedJob);
			break;
		}

		// ������ Job�� �ش��ϴ� JobQueue�� ���
		reservedJob->jobQueue->push(move(reservedJob->job));
	}

	_bDistributing.store(false);
}
