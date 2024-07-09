#pragma once

#include <thread>

class ThreadManager
{
private:
	static ThreadManager* _instance;

	ThreadManager();
	~ThreadManager();

public:
	static ThreadManager* instance();

	void launch(function<void(void)> callback);
	void join();

	void executeJobQueue();
	void distributeReservedJob();

private:
	mutex _mutex;
	vector<thread> _threads;
};

