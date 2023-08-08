#include "pch.h"
#include "ThreadManager.h"
#include "WinApi.h"
#include "PythonMap.h"

void ThreadManager::Push(std::function<void()> func)
{
	WRITE_LOCK;

	_threads.push_back(std::thread(
		[=]() 
		{ 
			InitTLS();
			func();
			DestroyTLS();
		}
	));
}

void ThreadManager::Join()
{
	for (int i = 0;i < static_cast<int32>(_threads.size());i++)
	{
		_threads[i].join();
	}
}

void ThreadManager::InitTLS()
{
	static std::atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);
	LMyThreadState = PyThreadState_New(MAP->mainThreadState->interp);
}

void ThreadManager::DestroyTLS()
{
	
}
