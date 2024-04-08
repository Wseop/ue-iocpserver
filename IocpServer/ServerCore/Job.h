#pragma once

using JobFunc = function<void(void)>;

class Job
{
public:
	Job(JobFunc&& jobFunc);

	template<typename Ret, typename T, typename... Args>
	inline Job(shared_ptr<T> owner, Ret(T::* func)(Args...), Args... args)
	{
		_jobFunc = [owner, func, args...]()
			{
				(owner.get()->*func)(args...);
			};
	}

public:
	void Execute();

private:
	JobFunc _jobFunc = nullptr;
};

