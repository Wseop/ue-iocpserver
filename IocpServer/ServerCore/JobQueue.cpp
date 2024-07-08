#include "pch.h"
#include "JobQueue.h"
#include "Job.h"

JobQueue::~JobQueue()
{
}

void JobQueue::push(shared_ptr<Job>&& job)
{
	if (job == nullptr)
		return;

	{
		lock_guard<mutex> lock(_mutex);

		// Job �߰�
		_jobs.push(job);

		// ť�� ����ִ� Job�� ó������ �ƴ϶�� flag ���� �� Global ť�� ���
		if (_bExecuting == false)
		{
			_bExecuting = true;
			gJobQueue->push(shared_from_this());
		}
	}
}

void JobQueue::execute()
{
	// ť�� �ִ� ��� Job�� ó��
	while (true)
	{
		shared_ptr<Job> job = nullptr;
		{
			lock_guard<mutex> lock(_mutex);

			// Job�� ������ flag ���� �� ����
			if (_jobs.empty())
			{
				_bExecuting = false;
				break;
			}

			job = _jobs.front();
			_jobs.pop();
		}
		job->execute();
	}
}
