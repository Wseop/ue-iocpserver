#include "pch.h"
#include "JobQueue.h"
#include "Job.h"

JobQueue::~JobQueue()
{
}

void JobQueue::push(shared_ptr<Job>&& job, bool bPushOnly)
{
	if (job == nullptr)
		return;

	uint32 prevCount = _jobCount.fetch_add(1);
	_jobs.push(job);

	// ���� ���� Push�� Thread�� Job ����
	if (prevCount == 0)
	{
		if (tJobQueue == nullptr && bPushOnly == false)
		{
			tJobQueue = shared_from_this();
			execute();
		}
		// �̹� ó������ JobQueue�� �ְų�, PushOnly�� ������ ���. �ٸ� Thread���� ó���� �� �ֵ��� global queue�� �߰�
		else
		{
			gJobQueue->push(shared_from_this());
		}
	}
}

void JobQueue::execute()
{
	// ��� Job ����
	uint32 executeCount = 0;
	shared_ptr<Job> job = nullptr;
	while (_jobs.try_pop(job))
	{
		job->execute();
		executeCount++;
	}

	// �����ִ� Job�� �� ������ �ٸ� Thread�� ó���� �� �ֵ��� GlobalQueue�� �߰�
	if (_jobCount.fetch_sub(executeCount) != executeCount)
		gJobQueue->push(shared_from_this());

	tJobQueue = nullptr;
}
