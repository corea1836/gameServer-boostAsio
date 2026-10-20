#include "Metrics.h"
#include "CoreManager.h"
#include "JobSerializer.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <sys/resource.h>

namespace
{

std::mutex GStrandWaitLock;
std::vector<int64> GStrandWaitUs;

int64 ToUs(std::chrono::steady_clock::duration duration)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

// 프로세스가 지금까지 쓴 CPU 시간(user + sys). 스레드 전체 합이다
int64 GetProcessCpuUs()
{
    rusage usage{};
    getrusage(RUSAGE_SELF, &usage);
    return (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1'000'000LL +
           usage.ru_utime.tv_usec + usage.ru_stime.tv_usec;
}

// libc++의 std::cout은 여러 스레드가 동시에 쓰면 TSan이 데이터 레이스(ios_base::width)로 잡는다.
// 지표 로그가 TSan 검증(1-4, 1-5)에 경고를 보태지 않도록 내부에서 락을 거는 stdio로 쓴다
void PrintLine(const std::string &line)
{
    std::printf("%s\n", line.c_str());
    std::fflush(stdout);
}

// p50/p99/max 형태로 쓴다
std::string FormatPercentiles(std::vector<int64> &samples)
{
    std::ostringstream out;
    out << Metrics::Percentile(samples, 50) << '/'
        << Metrics::Percentile(samples, 99) << '/'
        << Metrics::Percentile(samples, 100);
    return out.str();
}

} // namespace

void Metrics::RecordStrandWait(std::chrono::steady_clock::duration wait)
{
    std::lock_guard<std::mutex> guard(GStrandWaitLock);
    if (GStrandWaitUs.size() < MAX_STRAND_WAIT_SAMPLES)
        GStrandWaitUs.push_back(ToUs(wait));
}

std::vector<int64> Metrics::TakeStrandWaitSamplesUs()
{
    std::vector<int64> samples;
    std::lock_guard<std::mutex> guard(GStrandWaitLock);
    samples.swap(GStrandWaitUs);
    return samples;
}

int64 Metrics::Percentile(std::vector<int64> &samples, double p)
{
    if (samples.empty())
        return 0;

    // p * n을 먼저 곱해야 정수끼리 계산이 정확하다 (0.99 * 100은 99가 아닐 수 있다)
    size_t rank = static_cast<size_t>(std::ceil(p * samples.size() / 100.0));
    size_t index = std::min(rank == 0 ? 0 : rank - 1, samples.size() - 1);

    std::nth_element(samples.begin(), samples.begin() + index, samples.end());
    return samples[index];
}

void ZoneMetrics::RecordTick(std::chrono::steady_clock::duration elapsed)
{
    _tickUs.push_back(ToUs(elapsed));
}

void ZoneMetrics::ReportIfDue(int64 zoneId, int64 missCount, JobSerializer &jobs)
{
    auto now = std::chrono::steady_clock::now();
    if (now - _lastReportTime < std::chrono::milliseconds(Metrics::REPORT_INTERVAL_MS))
        return;
    _lastReportTime = now;

    // jobs가 MAX_WAIT_SAMPLES와 같으면 그 뒤 잡은 샘플에서 빠졌다
    std::vector<int64> waitUs = jobs.TakeWaitSamplesUs();

    std::ostringstream line;
    line << "[Metrics] zone=" << zoneId << " ticks=" << _tickUs.size()
         << " tick_us(p50/p99/max)=" << FormatPercentiles(_tickUs)
         << " miss=" << missCount - _lastMissCount << " jobs=" << waitUs.size()
         << " job_wait_us(p50/p99/max)=" << FormatPercentiles(waitUs);
    PrintLine(line.str());

    _tickUs.clear();
    _lastMissCount = missCount;
}

MetricsReporter::MetricsReporter(boost_io_context &ioContext)
    : _timer(ioContext)
{
}

void MetricsReporter::Start()
{
    _lastReportTime = std::chrono::steady_clock::now();
    _lastCpuUs = GetProcessCpuUs();
    _lastSentBytes = Metrics::SSentBytes.load();
    _lastRecvBytes = Metrics::SRecvBytes.load();
    _lastSendPackets = Metrics::SSendPackets.load();
    _lastWrites = Metrics::SWrites.load();

    AsyncWait();
}

void MetricsReporter::AsyncWait()
{
    _timer.expires_after(std::chrono::milliseconds(Metrics::REPORT_INTERVAL_MS));
    _timer.async_wait(
        [self = shared_from_this()](boost_error_code errorCode)
        {
            if (errorCode)
                return;

            self->Report();
            self->AsyncWait();
        });
}

void MetricsReporter::Report()
{
    auto now = std::chrono::steady_clock::now();
    int64 cpuUs = GetProcessCpuUs();
    int64 sentBytes = Metrics::SSentBytes.load();
    int64 recvBytes = Metrics::SRecvBytes.load();
    int64 sendPackets = Metrics::SSendPackets.load();
    int64 writes = Metrics::SWrites.load();
    int32 sessionCount = GSessionContainer.GetSessionCount();

    double seconds = ToUs(now - _lastReportTime) / 1'000'000.0;
    double sentPerSec = (sentBytes - _lastSentBytes) / seconds;
    double recvPerSec = (recvBytes - _lastRecvBytes) / seconds;
    // 구간 끝의 세션 수로 나눈 근사값. 구간 중에 접속/종료가 많으면 어긋난다
    double sentPerSession = sessionCount > 0 ? sentPerSec / sessionCount : 0;
    double recvPerSession = sessionCount > 0 ? recvPerSec / sessionCount : 0;

    // CPU 100% = 코어 하나를 다 쓴 것. 스레드가 여럿이라 100%를 넘을 수 있다
    double cpuPercent = (cpuUs - _lastCpuUs) / 10'000.0 / seconds;

    // 샘플 수가 MAX_STRAND_WAIT_SAMPLES와 같으면 그 뒤 쓰기는 샘플에서 빠졌다
    std::vector<int64> strandWaitUs = Metrics::TakeStrandWaitSamplesUs();

    std::ostringstream line;
    line << std::fixed << std::setprecision(1) << "[Metrics] sessions=" << sessionCount
         << " send_Bps=" << sentPerSec << " recv_Bps=" << recvPerSec
         << " send_Bps_per_session=" << sentPerSession
         << " recv_Bps_per_session=" << recvPerSession
         << " send_pkts_per_sec=" << (sendPackets - _lastSendPackets) / seconds
         << " writes_per_sec=" << (writes - _lastWrites) / seconds
         << " strand_waits=" << strandWaitUs.size()
         << " strand_wait_us(p50/p99/max)=" << FormatPercentiles(strandWaitUs)
         << " cpu=" << cpuPercent << '%';
    PrintLine(line.str());

    _lastReportTime = now;
    _lastCpuUs = cpuUs;
    _lastSentBytes = sentBytes;
    _lastRecvBytes = recvBytes;
    _lastSendPackets = sendPackets;
    _lastWrites = writes;
}
