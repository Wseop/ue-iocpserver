#include "pch.h"
#include "JobQueue.h"
#include "Job.h"

void JobQueue::Push(shared_ptr<Job> job, bool pushOnly)
{
	if (job == nullptr)
		return;

	_jobCount++;
	_jobs.Push(job);

	// 처리중인 JobQueue가 없다면 현재 JobQueue를 담당
	if (pushOnly == false && tJobQueue == nullptr)
	{
		tJobQueue = shared_from_this();
		Execute();
	}
	// 바로 처리할 수 없다면 다른 Thread에서 가져갈 수 있도록 Global Queue에 등록
	else
	{
		gJobQueue->Push(shared_from_this());
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
		gJobQueue->Push(shared_from_this());
	}

	tJobQueue = nullptr;
}
