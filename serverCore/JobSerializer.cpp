#include "JobSerializer.h"
#include "CoreManager.h"
#include "IoContext.h"
#include "TimeUtils.h"
#include <iostream>

JobSerializer::JobSerializer()
    : _gameIoContext(GIoContext.GetGameIoContext())
{
}

void JobSerializer::AddJob(JobRef job)
{
    bool isExecuting = false;
    {
        std::lock_guard<std::mutex> lockGuard(_lock);
        _jobs.push(std::move(job));

        if (_isExecuting.exchange(true) == false)
        {
            isExecuting = true;
        }
    }

    if (isExecuting)
    {
        boost::asio::post(_gameIoContext,
                          [self = shared_from_this()]() { self->Execute(); });
    }
}

void JobSerializer::Execute()
{
    int64 endTickCount = TimeUtils::GetTickCount64() + EXECUTE_MAX_DURATION_MS;

    while (true)
    {
        std::vector<JobRef> executeJobs;

        {
            std::lock_guard<std::mutex> lockGuard(_lock);

            if (_jobs.empty())
            {
                _isExecuting.store(false);
                return;
            }

            int jobCount = 0;
            while (!_jobs.empty() && jobCount < DRAIN_COUNT)
            {
                executeJobs.push_back(std::move(_jobs.front()));
                _jobs.pop();
                jobCount++;
            }
        }

        for (const JobRef &job : executeJobs)
            job->Execute();

        if (TimeUtils::GetTickCount64() > endTickCount)
        {
            post(_gameIoContext,
                 [self = shared_from_this()]() { self->Execute(); });
            return;
        }
    }
}
