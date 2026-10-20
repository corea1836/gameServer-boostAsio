#include <gtest/gtest.h>

#include "Metrics.h"
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

// nearest-rank: 1..100에서 p50은 50번째, p99는 99번째, p100은 최댓값
TEST(Metrics, PercentileIsNearestRank)
{
    std::vector<int64> samples(100);
    std::iota(samples.begin(), samples.end(), 1);
    std::shuffle(samples.begin(), samples.end(), std::mt19937(42));

    EXPECT_EQ(Metrics::Percentile(samples, 50), 50);
    EXPECT_EQ(Metrics::Percentile(samples, 99), 99);
    EXPECT_EQ(Metrics::Percentile(samples, 100), 100);
}

// 표본이 적으면 p99도 그 안의 값이어야 한다(범위를 벗어나 읽지 않는다)
TEST(Metrics, PercentileOfFewSamples)
{
    std::vector<int64> single{7};
    EXPECT_EQ(Metrics::Percentile(single, 50), 7);
    EXPECT_EQ(Metrics::Percentile(single, 99), 7);

    std::vector<int64> empty;
    EXPECT_EQ(Metrics::Percentile(empty, 99), 0);
}
