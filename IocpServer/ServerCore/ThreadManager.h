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
	static void ExecuteJob();
	static void DistributeReservedJob();

private:
	mutex _mutex;
	vector<thread> _threads;
};

