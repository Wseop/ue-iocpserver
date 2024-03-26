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

	// ���� ó�� ������ Thread�� ������ ���
	if (prevCount == 0)
	{
		if (tJobQueue == nullptr)
		{
			tJobQueue = shared_from_this();
			Execute();
		}
		// �̹� �ٸ� JobQueue�� ��� �־��ٸ� �ٸ� Thread�� ������ �� �ֵ��� GlobalQueue�� �߰�
		else
		{
			gJobQueue->Push(shared_from_this());
		}
	}
}

void JobQueue::Execute()
{
	// ��� Job ����
	uint32 executeCount = 0;
	shared_ptr<Job> job = nullptr;
	while (_jobs.try_pop(job))
	{
		job->Execute();
		executeCount++;
	}

	// �����ִ� Job�� �� ������ �ٸ� Thread�� ó���� �� �ֵ��� GlobalQueue�� �߰�
	if (_jobCount.fetch_sub(executeCount) != executeCount)
		gJobQueue->Push(shared_from_this());

	tJobQueue = nullptr;
}
