#include "pch.h"
#include "JobQueue.h"

void JobQueue::Execute()
{
	vector<JobRef> jobs;
	_jobQueue.PopAll(jobs);

	for (JobRef job : jobs)
	{
		job->Excute();
	}
}