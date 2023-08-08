#pragma once
#include <functional>
#include <thread>

class ThreadManager
{
	SINGLETON(ThreadManager)

public:
	void Init() { InitTLS(); _threads.reserve(100); }
	void InitTLS();
	void DestroyTLS();
	void Push(std::function<void()> func);
	void Join();

private:
	USE_LOCK;
	std::vector<std::thread> _threads;
};

