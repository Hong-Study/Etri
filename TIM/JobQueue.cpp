#include "pch.h"
#include "JobQueue.h"

void JobQueue::Execute()
{
	uint64 tick = GetTickCount64();

	while (isWork)
	{
		uint64 now = GetTickCount64();
		if (now - tick >= 200)
		{
			tick = now;
			vector<JobRef> jobs;
			_jobQueue.PopAll(jobs);

			for (JobRef job : jobs)
			{
				job->Excute();
			}
		}
	}
}