#include "JobSerializer.h"
#include "TimeUtils.h"
#include "Types.h"
#include <chrono>
#include <thread>

JobSerializer::JobSerializer(boost_io_context &io_context)
    : _gameIoContext(io_context) {}

JobSerializer::~JobSerializer() {}

void JobSerializer::Push(JobRef &&job) {
    const int32 prevCount = _jobCount.fetch_add(1);

    //  cout << "Push Job" << endl;

    _jobs.Push(job);
    if (prevCount == 0) {
        TryExecute();
    } else {
        //        cout << "Skip TryExecute. Already Executing, PrevCount : " <<
        //        prevCount
        //             << endl;
    }
}

void JobSerializer::TryExecute() {
    ///    cout << "TryExecute Start" << endl;

    if (_executing.exchange(true)) {
        //       cout << "Already Executing.." << endl;
        return;
    }

    // cout << "Posting To GameIoContext" << endl;

    post(_gameIoContext, [this, self = shared_from_this()]() {
        //        cout << "Lambda called in game worker thread" << endl;
        Execute();
    });
    //    cout << "Post Completed." << endl;
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

        // this_thread::sleep_for(std::chrono::seconds(5));

        if (_jobCount.fetch_sub(jobCount) == jobCount) {
            _executing.store(false);
            return;
        }

        const uint64 now = TimeUtils::GetTickCount64();
        if (now > EndTickCount) {
            _executing.store(false);

            cout << "[Thread " << threadId
                 << "] Because of Timeout. Jobs Remain " << _jobs.Size()
                 << endl;

            post(_gameIoContext,
                 [this, self = shared_from_this()]() { TryExecute(); });
            return;
        }
    }
}
