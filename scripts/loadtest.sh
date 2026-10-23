#!/usr/bin/env bash
# 부하 측정(1-11). Release 빌드로 서버와 봇을 같은 머신에 띄우고 서버의 [Metrics] 로그를 모은다.
# 사용법: [DURATION_SEC=초] [OUT_DIR=경로] scripts/loadtest.sh [봇 수 ...]   (기본: 50 100 200, 봇 수마다 30초)
# 로그: build/loadtest/server-<봇 수>.log, build/loadtest/bot-<봇 수>.log
set -euo pipefail

cd "$(dirname "$0")/.."

BUILD_DIR=build/release
OUT_DIR=${OUT_DIR:-build/loadtest}
DURATION_SEC=${DURATION_SEC:-30}

COUNTS=("$@")
if [ ${#COUNTS[@]} -eq 0 ]; then
    COUNTS=(50 100 200)
fi

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release > /dev/null
cmake --build "$BUILD_DIR" --target GameServer DummyClient > /dev/null
mkdir -p "$OUT_DIR"

for count in "${COUNTS[@]}"; do
    "$BUILD_DIR/gameServer/GameServer" > "$OUT_DIR/server-$count.log" 2>&1 &
    server_pid=$!
    sleep 1

    if ! "$BUILD_DIR/gameServer/DummyClient" "$count" "$DURATION_SEC" > "$OUT_DIR/bot-$count.log" 2>&1; then
        echo "!! 봇 $count: 모든 봇이 S_EnterWorld를 받지 못했다 ($OUT_DIR/bot-$count.log)"
    fi

    # 봇이 한꺼번에 끊길 때 서버가 먼저 죽을 수 있다(1-5). 이미 죽었으면 넘어간다
    kill "$server_pid" 2> /dev/null || echo "!! 봇 $count: 서버가 측정 끝에 이미 종료돼 있었다"
    wait "$server_pid" 2> /dev/null || true

    echo "== 봇 $count"
    grep "^\[Metrics\]" "$OUT_DIR/server-$count.log" || true
    grep "^\[Bot\]" "$OUT_DIR/bot-$count.log" | tail -1 || true
done
