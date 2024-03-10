#include "pch.h"
#include "JobQueue.h"
#include "Job.h"

void JobQueue::Push(shared_ptr<Job> job, bool pushOnly)
{
	if (job == nullptr)
		return;

	_jobCount++;
	_jobs.Push(job);

	// ó������ JobQueue�� ���ٸ� ���� JobQueue�� ���
	if (pushOnly == false && LJobQueue == nullptr)
	{
		LJobQueue = shared_from_this();
		Execute();
	}
	// �ٷ� ó���� �� ���ٸ� �ٸ� Thread���� ������ �� �ֵ��� Global Queue�� ���
	else
	{
		GJobQueue->Push(shared_from_this());
	}
}

void JobQueue::Execute()
{
	// ��� Job ������ ó��
	vector<shared_ptr<Job>> jobs;
	_jobs.PopAll(jobs);

	uint32 jobCount = static_cast<uint32>(jobs.size());

	for (uint32 i = 0; i < jobCount; i++)
	{
		jobs[i]->Execute();
	}

	// Job�� ���������� �ٸ� Thread���� ó���� �� �ֵ��� GlobalQueue�� ���
	if (_jobCount.fetch_sub(jobCount) != jobCount)
	{
		GJobQueue->Push(shared_from_this());
	}

	LJobQueue = nullptr;
}
