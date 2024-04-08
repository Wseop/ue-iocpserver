#include "pch.h"
#include "JobQueue.h"
#include "Job.h"

JobQueue::~JobQueue()
{
}

void JobQueue::Push(shared_ptr<Job>&& job, bool bPushOnly)
{
	if (job == nullptr)
		return;

	uint32 prevCount = _jobCount.fetch_add(1);
	_jobs.push(job);

	// 가장 먼저 Push한 Thread가 Job 실행
	if (prevCount == 0)
	{
		if (tJobQueue == nullptr && bPushOnly == false)
		{
			tJobQueue = shared_from_this();
			Execute();
		}
		// 이미 처리중인 JobQueue가 있거나, PushOnly로 설정된 경우. 다른 Thread에서 처리할 수 있도록 global queue에 추가
		else
		{
			gJobQueue->push(shared_from_this());
		}
	}
}

void JobQueue::Execute()
{
	// 모든 Job 실행
	uint32 executeCount = 0;
	shared_ptr<Job> job = nullptr;
	while (_jobs.try_pop(job))
	{
		job->Execute();
		executeCount++;
	}

	// 남아있는 Job이 더 있으면 다른 Thread가 처리할 수 있도록 GlobalQueue에 추가
	if (_jobCount.fetch_sub(executeCount) != executeCount)
		gJobQueue->push(shared_from_this());

	tJobQueue = nullptr;
}
