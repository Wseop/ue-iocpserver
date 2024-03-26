#include "pch.h"
#include "JobQueue.h"
#include "Job.h"

JobQueue::~JobQueue()
{
}

void JobQueue::Push(shared_ptr<Job> job, bool pushOnly)
{
	if (job == nullptr)
		return;

	const uint32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job);

	// ���� ���� Job�� Push�� Thread�� ������ ���
	if (prevCount == 0)
	{
		// ó������ JobQueue�� ���ٸ� ���� JobQueue�� ���
		if (pushOnly == false && tJobQueue == nullptr)
		{
			tJobQueue = shared_from_this();
			Execute();
		}
		// �ٷ� ó���� �� ���ٸ� �ٸ� Thread���� ������ �� �ֵ��� Global Queue�� ���
		else
		{
			gJobQueue->Push(shared_from_this());
		}
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
		gJobQueue->Push(shared_from_this());
	}

	tJobQueue = nullptr;
}
