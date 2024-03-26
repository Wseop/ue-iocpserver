#include "pch.h"
#include "JobQueue.h"
#include "Job.h"

JobQueue::~JobQueue()
{
}

void JobQueue::Push(shared_ptr<Job> job)
{
	if (job == nullptr)
		return;

	uint32 prevCount = _jobCount.fetch_add(1);
	_jobs.push(job);

	// 가장 처음 접근한 Thread가 실행을 담당
	if (prevCount == 0)
	{
		if (tJobQueue == nullptr)
		{
			tJobQueue = shared_from_this();
			Execute();
		}
		// 이미 다른 JobQueue를 잡고 있었다면 다른 Thread가 가져갈 수 있도록 GlobalQueue에 추가
		else
		{
			gJobQueue->Push(shared_from_this());
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
		gJobQueue->Push(shared_from_this());

	tJobQueue = nullptr;
}
