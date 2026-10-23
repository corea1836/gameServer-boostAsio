#include <gtest/gtest.h>

#include "Visibility.h"
#include <algorithm>

// AOI 1단계 가시성 규칙(ADR-022). 반경은 튜닝 값이라 여기서는 임의의 값(5, 10)을 쓴다
//   안 보이던 대상: 지면 거리 <= ENTER면 들어온다
//   보이던 대상:   지면 거리 > LEAVE이거나 월드에서 사라지면 나간다
namespace
{
    constexpr float ENTER = 5.f;
    constexpr float LEAVE = 10.f;
    constexpr int64 SELF = 1;
    const Vector3 ORIGIN{0, 0, 0};

    std::vector<int64> Sorted(std::vector<int64> ids)
    {
        std::sort(ids.begin(), ids.end());
        return ids;
    }

    std::vector<int64> Sorted(const std::unordered_set<int64> &ids)
    {
        return Sorted(std::vector<int64>(ids.begin(), ids.end()));
    }
}

TEST(VisibilityTest, EntersWithinEnterRadius)
{
    std::unordered_set<int64> visible;

    VisibilityDiff diff = UpdateVisibility(visible, SELF, ORIGIN, {{SELF, ORIGIN}, {2, {3, 0, 0}}}, ENTER, LEAVE);

    EXPECT_EQ(diff.entered, std::vector<int64>{2});
    EXPECT_TRUE(diff.left.empty());
    EXPECT_EQ(Sorted(visible), std::vector<int64>{2});
}

TEST(VisibilityTest, DoesNotEnterBetweenRadii)
{
    std::unordered_set<int64> visible;

    VisibilityDiff diff = UpdateVisibility(visible, SELF, ORIGIN, {{2, {7, 0, 0}}}, ENTER, LEAVE);

    EXPECT_TRUE(diff.entered.empty());
    EXPECT_TRUE(diff.left.empty());
    EXPECT_TRUE(visible.empty());
}

TEST(VisibilityTest, StaysVisibleBetweenRadii)
{
    std::unordered_set<int64> visible{2};

    VisibilityDiff diff = UpdateVisibility(visible, SELF, ORIGIN, {{2, {7, 0, 0}}}, ENTER, LEAVE);

    EXPECT_TRUE(diff.entered.empty());
    EXPECT_TRUE(diff.left.empty());
    EXPECT_EQ(Sorted(visible), std::vector<int64>{2});
}

TEST(VisibilityTest, LeavesBeyondLeaveRadius)
{
    std::unordered_set<int64> visible{2};

    VisibilityDiff diff = UpdateVisibility(visible, SELF, ORIGIN, {{2, {11, 0, 0}}}, ENTER, LEAVE);

    EXPECT_TRUE(diff.entered.empty());
    EXPECT_EQ(diff.left, std::vector<int64>{2});
    EXPECT_TRUE(visible.empty());
}

// 들어오기는 "<= ENTER", 나가기는 "> LEAVE"라서 거리가 딱 반경이면 들어오고, 보이던 대상은 남는다
TEST(VisibilityTest, BoundaryDistanceEntersAndStays)
{
    std::unordered_set<int64> visible{3};

    // (3, 4)는 거리 5, (6, 8)은 거리 10
    VisibilityDiff diff = UpdateVisibility(visible, SELF, ORIGIN, {{2, {3, 4, 0}}, {3, {6, 8, 0}}}, ENTER, LEAVE);

    EXPECT_EQ(diff.entered, std::vector<int64>{2});
    EXPECT_TRUE(diff.left.empty());
    EXPECT_EQ(Sorted(visible), (std::vector<int64>{2, 3}));
}

TEST(VisibilityTest, IgnoresSelf)
{
    std::unordered_set<int64> visible;

    VisibilityDiff diff = UpdateVisibility(visible, SELF, ORIGIN, {{SELF, ORIGIN}}, ENTER, LEAVE);

    EXPECT_TRUE(diff.entered.empty());
    EXPECT_TRUE(visible.empty());
}

// 월드를 떠난 대상은 candidates에서 빠진다. 보고 있었다면 거리와 상관없이 나간다
TEST(VisibilityTest, LeavesWhenRemovedFromCandidates)
{
    std::unordered_set<int64> visible{2};

    VisibilityDiff diff = UpdateVisibility(visible, SELF, ORIGIN, {{SELF, ORIGIN}}, ENTER, LEAVE);

    EXPECT_EQ(diff.left, std::vector<int64>{2});
    EXPECT_TRUE(visible.empty());
}

// 높이(z)는 거리에 넣지 않는다(ADR-010 2.5D)
TEST(VisibilityTest, IgnoresHeight)
{
    std::unordered_set<int64> visible;

    VisibilityDiff diff = UpdateVisibility(visible, SELF, ORIGIN, {{2, {3, 0, 100}}}, ENTER, LEAVE);

    EXPECT_EQ(diff.entered, std::vector<int64>{2});
}

TEST(VisibilityTest, MeasuresFromSelfPosition)
{
    std::unordered_set<int64> visible;
    const Vector3 selfPos{100, 100, 0};

    VisibilityDiff diff = UpdateVisibility(visible, SELF, selfPos, {{2, {103, 100, 0}}, {3, {3, 0, 0}}}, ENTER, LEAVE);

    EXPECT_EQ(diff.entered, std::vector<int64>{2});
    EXPECT_EQ(Sorted(visible), std::vector<int64>{2});
}

// 들어오고, 남고, 나가고, 안 들어오는 대상이 한 번에 섞여 있다. 이미 보이는 대상은 다시 entered에 넣지 않는다
TEST(VisibilityTest, MixedTargetsInOneUpdate)
{
    std::unordered_set<int64> visible{2, 3};

    VisibilityDiff diff = UpdateVisibility(
        visible, SELF, ORIGIN, {{2, {1, 0, 0}}, {3, {20, 0, 0}}, {4, {2, 0, 0}}, {5, {30, 0, 0}}}, ENTER, LEAVE);

    EXPECT_EQ(Sorted(diff.entered), std::vector<int64>{4});
    EXPECT_EQ(Sorted(diff.left), std::vector<int64>{3});
    EXPECT_EQ(Sorted(visible), (std::vector<int64>{2, 4}));
}

// 들어오는 반경 경계를 안팎으로 오가도 spawn/despawn이 반복되지 않는다(히스테리시스의 목적)
TEST(VisibilityTest, NoFlickerAcrossEnterRadius)
{
    std::unordered_set<int64> visible;
    int32 enteredCount = 0;
    int32 leftCount = 0;

    for (int32 i = 0; i < 10; i++)
    {
        float x = (i % 2 == 0) ? 4.9f : 5.1f;
        VisibilityDiff diff = UpdateVisibility(visible, SELF, ORIGIN, {{2, {x, 0, 0}}}, ENTER, LEAVE);
        enteredCount += static_cast<int32>(diff.entered.size());
        leftCount += static_cast<int32>(diff.left.size());
    }

    EXPECT_EQ(enteredCount, 1);
    EXPECT_EQ(leftCount, 0);
}
