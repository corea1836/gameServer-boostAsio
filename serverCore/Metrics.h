#pragma once

#include "BoostTypes.h"
#include "Types.h"
#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

class JobSerializer;

// 1-8 서버 지표. REPORT_INTERVAL_MS마다 한 줄씩 로그로 남긴다
class Metrics
{
    public:
        static constexpr int64 REPORT_INTERVAL_MS = 5000;

        // 모든 세션의 송수신 바이트 합. 네트워크 스레드 여럿이 더한다
        static inline std::atomic<int64> SSentBytes = 0;
        static inline std::atomic<int64> SRecvBytes = 0;
        // Send 호출 수(패킷) 대비 실제 async_write 횟수. 차이만큼 쓰기가 묶여 나갔다
        static inline std::atomic<int64> SSendPackets = 0;
        static inline std::atomic<int64> SWrites = 0;

        // 1-4 세션 strand: Write를 post한 뒤 strand에서 실행되기까지 걸린 시간.
        // 여러 network 스레드가 넣는다. 구간마다 MAX_STRAND_WAIT_SAMPLES개까지만 모은다
        static constexpr size_t MAX_STRAND_WAIT_SAMPLES = 65536;
        static void RecordStrandWait(std::chrono::steady_clock::duration wait);
        static std::vector<int64> TakeStrandWaitSamplesUs();

        // nearest-rank 백분위(p는 0~100). samples의 순서를 바꾼다. 비어 있으면 0
        static int64 Percentile(std::vector<int64> &samples, double p);
};

// 존 하나의 틱 시간, 데드라인 초과, 잡 대기시간. 그 존의 JobSerializer 위에서만 쓴다
class ZoneMetrics
{
    public:
        void RecordTick(std::chrono::steady_clock::duration elapsed);
        // 마지막 보고 뒤 REPORT_INTERVAL_MS가 지났으면 로그를 남기고 구간을 비운다.
        // playerCount는 존 인원이다. 봇이 다 끊긴 뒤에도 0이 아니면 유령 플레이어가 남은 것이다(1-5)
        void ReportIfDue(int64 zoneId, int64 playerCount, int64 missCount, JobSerializer &jobs);

    private:
        std::vector<int64> _tickUs;
        int64 _lastMissCount = 0;
        std::chrono::steady_clock::time_point _lastReportTime =
            std::chrono::steady_clock::now();
};

// 세션 수, 세션 송수신 바이트, 프로세스 CPU를 REPORT_INTERVAL_MS마다 남긴다
class MetricsReporter : public std::enable_shared_from_this<MetricsReporter>
{
    public:
        MetricsReporter(boost_io_context &ioContext);

        void Start();

    private:
        void AsyncWait();
        void Report();

        boost_timer _timer;
        std::chrono::steady_clock::time_point _lastReportTime;
        int64 _lastSentBytes = 0;
        int64 _lastRecvBytes = 0;
        int64 _lastSendPackets = 0;
        int64 _lastWrites = 0;
        int64 _lastCpuUs = 0;
};
