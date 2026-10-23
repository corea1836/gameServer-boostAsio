#include "Visibility.h"

VisibilityDiff UpdateVisibility(
    std::unordered_set<int64> &visible,
    int64 selfId, const Vector3 &selfPos,
    const std::vector<std::pair<int64, Vector3>> &candidates,
    float enterRadius, float leaveRadius)
{
    VisibilityDiff diff;
    std::unordered_set<int64> next;

    for (auto &[id, pos] : candidates)
    {
        if (id == selfId)
            continue;

        float dx = pos.x - selfPos.x;
        float dy = pos.y - selfPos.y;
        float distanceSq = dx * dx + dy * dy;

        // 보이던 대상은 나가는 반경, 안 보이던 대상은 들어오는 반경으로 판단한다(히스테리시스)
        bool wasVisible = visible.count(id) > 0;
        float radius = wasVisible ? leaveRadius : enterRadius;
        if (distanceSq > radius * radius)
            continue;

        next.insert(id);
        if (!wasVisible)
            diff.entered.push_back(id);
    }

    // 멀어졌거나 candidates에서 사라진(월드를 떠난) 대상
    for (int64 id : visible)
    {
        if (next.count(id) == 0)
            diff.left.push_back(id);
    }

    visible = std::move(next);
    return diff;
}
