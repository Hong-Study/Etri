#pragma once
#include <functional>
#include <utility>

using CallbackType = std::function<void()>;
using JobRef = shared_ptr<class Job>;

class Job
{
public:
	Job(CallbackType&& callback) : _callback(std::move(callback))
	{

	}

	template<typename T, typename Ret, typename... Args>
	Job(shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		_callback = [owner, memFunc, args...]()
		{
			(owner.get()->*memFunc)(args...);
		};
	}

	void Excute()
	{
		_callback();
	}
	
private:
	CallbackType _callback;
};

template<typename T>
class LockQueue
{
public:
	void Push(T item)
	{
		WRITE_LOCK;
		_items.push(item);
	}

	T Pop()
	{
		WRITE_LOCK;
		if (_items.empty())
			return T();

		T ret = _items.front();
		_items.pop();
		return ret;
	}

	void PopAll(OUT std::vector<T>& items)
	{
		WRITE_LOCK;
		while (T item = Pop())
			items.push_back(item);
	}

	void Clear()
	{
		WRITE_LOCK;
		_items = std::queue<T>();
	}

private:
	USE_LOCK;
	std::queue<T> _items;
};

class JobQueue
{
public:
	void DoAsync(CallbackType&& callback)
	{
		JobRef job = make_shared<Job>(callback);
		Push(job);
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::*memFunc)(Args...), Args&&... args)
	{
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		JobRef job = make_shared<Job>(owner, memFunc, std::forward<Args>(args)...);
		Push(job);
	}

	void ClearJobs() { _jobQueue.Clear(); }

public:
	void Push(JobRef job)
	{
		_jobQueue.Push(job);
	}
	void Execute();

	LockQueue<JobRef> _jobQueue;
};
