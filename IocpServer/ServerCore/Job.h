#pragma once

using JobFunc = function<void(void)>;

class Job
{
public:
	inline Job(JobFunc&& jobFunc) : _jobFunc(jobFunc) {}

	template<typename T, typename Ret, typename... Args>
	inline Job(shared_ptr<T> owner, Ret(T::*func)(Args...), Args... args)
	{
		_jobFunc = [owner, func, args...]()
			{
				(owner.get()->*func)(args...);
			};
	}

	inline void Execute()
	{
		_jobFunc();
	}

private:
	JobFunc _jobFunc = nullptr;
};

