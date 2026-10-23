#pragma once

#include "MathUtils.h"
#include <Types.h>
#include <unordered_set>
#include <utility>
#include <vector>

// AOI 1단계(ADR-022): 셀 인덱스 없이 전수 거리 비교 + 원형 반경 히스테리시스
struct VisibilityDiff
{
        std::vector<int64> entered;
        std::vector<int64> left;
};

// visible: 이 플레이어가 지금 보고 있는 id (갱신됨)
// candidates: 이번 틱 월드의 전원 (id, 위치)
// 들어오기: 거리 <= enterRadius
// 나가기: 거리 > leaveRadius, 또는 candidates에 없음
// 거리는 지면(x, y)만 본다
VisibilityDiff UpdateVisibility(
    std::unordered_set<int64> &visible,
    int64 selfId, const Vector3 &selfPos,
    const std::vector<std::pair<int64, Vector3>> &candidates,
    float enterRadius, float leaveRadius);
