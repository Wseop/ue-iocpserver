#include "pch.h"
#include "Job.h"

Job::Job(JobFunc&& jobFunc) : _jobFunc(move(jobFunc))
{
}

void Job::Execute()
{
	if (_jobFunc != nullptr)
		_jobFunc();
}
