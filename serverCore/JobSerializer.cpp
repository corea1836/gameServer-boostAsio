#include "JobSerializer.h"

JobSerializer::JobSerializer(boost_io_context &io_context)
    : _gameIoContext(io_context) {}

JobSerializer::~JobSerializer() {}

void JobSerializer::Push(JobRef &&job) {
    const int32 prevCount = _jobCount.fetch_add(1);

    _jobs.Push(job);
    if (prevCount == 0) {
        TryExecute();
    } else {
    }
}

void JobSerializer::TryExecute() {

    if (_executing.exchange(true)) {
        return;
    }

    post(_gameIoContext, [this, self = shared_from_this()]() { Execute(); });
}

void JobSerializer::Execute() {
    auto threadId = this_thread::get_id();
    auto currentTickCount = TimeUtils::GetTickCount64();

    cout << "[Thread " << threadId
         << "] Execute Jobs. currentTickCount : " << currentTickCount << endl;

    uint64 EndTickCount = currentTickCount + 64;
    while (true) {
        vector<JobRef> jobs;
        _jobs.Pop(jobs, 100);

        const int32 jobCount = static_cast<int32>(jobs.size());
        for (int32 i = 0; i < jobCount; i++) {
            jobs[i]->Execute();
        }

        if (_jobCount.fetch_sub(jobCount) == jobCount) {
            _executing.store(false);

            if (_jobCount.load() > 0) {
                TryExecute();
            }
            return;
        }

        const uint64 now = TimeUtils::GetTickCount64();
        if (now > EndTickCount) {
            _executing.store(false);

            cout << "[Thread " << threadId
                 << "] Because of Timeout. Jobs Remain " << _jobs.Size()
                 << endl;

            TryExecute();
            return;
        }
    }
}
