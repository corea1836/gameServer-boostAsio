#!/usr/bin/env bash
# 세션 close 경쟁 검증(1-4, 1-5). TSan 서버에 움직이는 봇(DummyClient)과 접속/종료 반복 봇(ChurnClient)을 붙인다.
# 움직이는 봇은 실행 초가 끝나면 한꺼번에 끊긴다(대량 종료). 끝나면 서버의 TSan 경고를 위치별로 묶어 보여 준다.
# 사용법: scripts/churntest.sh [실행 초=120] [움직이는 봇 수=50] [반복 봇 수=50]
#   T1_CHURN_BUILD=release 이면 TSan 없이 Release 서버로 돌린다(크래시 재현용)
# 봇은 Release로 돌린다. 봇까지 TSan이면 느려져서 서버에 걸리는 부하가 줄어든다
# 로그: build/churntest/server.log, move.log, churn.log
# 종료 코드: 서버가 도중에 죽었거나 TSan 경고가 있으면 1
set -euo pipefail

cd "$(dirname "$0")/.."

DURATION_SEC=${1:-120}
MOVE_BOTS=${2:-50}
CHURN_BOTS=${3:-50}
OUT_DIR=build/churntest

if [ "${T1_CHURN_BUILD:-tsan}" = release ]; then
    SERVER_BUILD_DIR=build/release
    cmake -S . -B "$SERVER_BUILD_DIR" -DCMAKE_BUILD_TYPE=Release > /dev/null
else
    SERVER_BUILD_DIR=build/tsan
    cmake -S . -B "$SERVER_BUILD_DIR" -DT1_SANITIZE=thread > /dev/null
fi
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release > /dev/null
cmake --build "$SERVER_BUILD_DIR" --target GameServer > /dev/null
cmake --build build/release --target DummyClient ChurnClient > /dev/null
mkdir -p "$OUT_DIR"

# 경고가 나도 서버를 멈추지 않는다
TSAN_OPTIONS="halt_on_error=0 ${TSAN_OPTIONS:-}" \
    "$SERVER_BUILD_DIR/gameServer/GameServer" > "$OUT_DIR/server.log" 2>&1 &
server_pid=$!
sleep 2

build/release/gameServer/DummyClient "$MOVE_BOTS" "$DURATION_SEC" > "$OUT_DIR/move.log" 2>&1 &
move_pid=$!
build/release/gameServer/ChurnClient "$CHURN_BOTS" "$DURATION_SEC" > "$OUT_DIR/churn.log" 2>&1 &
churn_pid=$!

wait "$move_pid" || true
wait "$churn_pid" || true
# 서버가 대량 종료를 처리할 시간
sleep 3

failed=0
if kill -0 "$server_pid" 2> /dev/null; then
    kill "$server_pid"
    wait "$server_pid" 2> /dev/null || true
else
    wait "$server_pid" && status=0 || status=$?
    echo "!! 서버가 도중에 죽었다 (종료 코드 $status, $OUT_DIR/server.log)"
    failed=1
fi

echo "== 반복 봇"
grep "^\[Churn\]" "$OUT_DIR/churn.log" | tail -n 1 || true
echo "== 움직이는 봇"
grep "^\[Bot\]" "$OUT_DIR/move.log" | tail -n 1 || true

warnings=$(grep -c "^WARNING: ThreadSanitizer" "$OUT_DIR/server.log" || true)
echo "== TSan 경고 ${warnings}건 (위치별)"
grep "^SUMMARY: ThreadSanitizer" "$OUT_DIR/server.log" | sed "s|$PWD/||" | sort | uniq -c | sort -rn || true

if [ "$warnings" -gt 0 ]; then
    failed=1
fi
exit "$failed"
