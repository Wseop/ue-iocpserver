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

		// Job 추가
		_jobs.push(job);

		// 큐에 들어있는 Job을 처리중이 아니라면 flag 변경 및 Global 큐에 등록
		if (_bExecuting == false)
		{
			_bExecuting = true;
			gJobQueue->push(shared_from_this());
		}
	}
}

void JobQueue::execute()
{
	// 큐에 있는 모든 Job을 처리
	while (true)
	{
		shared_ptr<Job> job = nullptr;
		{
			lock_guard<mutex> lock(_mutex);

			// Job이 없으면 flag 변경 및 종료
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
