#include "pch.h"
#include "JobQueue.h"
#include "Job.h"

void JobQueue::Push(shared_ptr<Job> job, bool pushOnly)
{
	_jobCount++;
	_jobs.Push(job);

	// 처리중인 JobQueue가 없다면 현재 JobQueue를 담당
	if (pushOnly == false && LJobQueue == nullptr)
	{
		LJobQueue = shared_from_this();
		Execute();
	}
}

void JobQueue::Execute()
{
	// 모든 Job 꺼내서 처리
	vector<shared_ptr<Job>> jobs;
	_jobs.PopAll(jobs);

	uint32 jobCount = static_cast<uint32>(jobs.size());

	for (uint32 i = 0; i < jobCount; i++)
	{
		jobs[i]->Execute();
	}

	// Job이 남아있으면 다른 Thread에서 처리될 수 있도록 GlobalQueue에 등록
	if (_jobCount.fetch_sub(jobCount) != jobCount)
	{
		GJobQueue->Push(shared_from_this());
	}

	LJobQueue = nullptr;
}
