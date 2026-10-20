#include "JobSerializer.h"
#include "TimeUtils.h"
#include <iostream>

JobSerializer::JobSerializer(boost_io_context& gameIoContext)
    : _gameIoContext(gameIoContext)
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
        {
            std::lock_guard<std::mutex> lockGuard(_lock);

            if (_jobs.empty() && _executeJobs.empty())
            {
                _isExecuting.store(false);
                return;
            }

            int jobCount = 0;
            while (!_jobs.empty() && jobCount < DRAIN_COUNT)
            {
                _executeJobs.push_back(std::move(_jobs.front()));
                _jobs.pop();
                jobCount++;
            }
        }

        while (!_executeJobs.empty())
        {
            if (TimeUtils::GetTickCount64() > endTickCount)
            {
                post(_gameIoContext, [self = shared_from_this()] { self->Execute(); });
                return;
            }
            const JobRef job = std::move(_executeJobs.front());
            _executeJobs.pop_front();

            if (_waitSamplesUs.size() < MAX_WAIT_SAMPLES)
            {
                _waitSamplesUs.push_back(
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::steady_clock::now() - job->GetPostTime())
                        .count());
            }

            job->Execute();
        }
    }
}
