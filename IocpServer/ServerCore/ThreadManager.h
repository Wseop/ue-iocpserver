#pragma once

#include <thread>

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

public:
	void Launch(function<void(void)> callback);
	void Join();

public:
	static void InitTLS();
	static void ExecuteJobQueue();

private:
	mutex _mutex;
	vector<thread> _threads;
};

