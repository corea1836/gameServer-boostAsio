# 프로젝트: T1 액션 MMORPG 서버

## 기술 스택
- C++17, CMake 3.15 이상
- Boost.Asio (Homebrew boost 1.89), Protobuf 3.21 (Homebrew)
- 테스트: GoogleTest 1.17 (FetchContent), ThreadSanitizer (`-DT1_SANITIZE=thread`)
- 클라이언트: Unity (`~/Workspace/Unity_Projects/T1`, 이 저장소 밖). proto 원본은 그 안의 `Common/Protocol`

## 아키텍처 규칙
- CRITICAL: 학습 대상 파트는 Claude가 구현하지 않는다. `docs/PRD.md` 작업 표에서 담당이 "사용자" 또는 "Claude 테스트 → 사용자"인 작업은, Claude는 과제 설명, (규칙 로직이면) 실패하는 테스트 작성, 리뷰만 한다. 이유: 이 프로젝트는 학습이 최우선이다
- CRITICAL: 존(`World`)의 게임 상태는 그 존의 JobSerializer 위에서만 읽고 쓴다. 네트워크 스레드에서 직접 건드리지 마라. 이유: 존 상태의 소유자를 하나로 유지해 락 없이 일관성을 지킨다
- CRITICAL: proto 메시지 선언 순서를 바꾸지 마라. 이유: C# 쪽은 선언 순서로 패킷 ID를 매겨서, 순서가 바뀌면 ID가 조용히 어긋난다. R3(proto 자동화) 전까지 `gameServer/PacketHandler.h`의 ID enum은 손으로 맞춘다
- 처음부터 완성형을 넣지 않는다. 문제를 먼저 겪고, 측정하고, 그다음 기법을 도입한다(오버엔지니어링 금지)
- 어필·성능 주장에는 `docs/measurements.md`의 측정값이나 코드 근거가 있는 것만 쓴다
- 새 단계(AOI, 몬스터, 스킬 등)는 시작 전에 grilling으로 설계를 정하고 `docs/ADR.md`에 기록한다

## 개발 프로세스
- CRITICAL: 검증 방식은 로직 종류에 따라 나눈다 (ADR-016)
  - 규칙 로직(명세가 먼저 정해지는 것: 파싱, 핸들러 방어, 스케줄링, 좌표·충돌 변환, 길찾기, 판정 도형, 데미지·쿨다운 공식, 인벤토리): 테스트를 먼저 작성하고, 통과하는 구현을 작성한다 (TDD, GoogleTest)
  - 감각·튜닝 로직(플레이해 봐야 정해지는 것: 속도, 보정·보간 수치, AI 행동, 보스 패턴, 텔레그래프·관대화 수치): 테스트를 먼저 쓰지 마라. 플레이와 측정으로 값을 정한 뒤, 깨지면 안 되는 불변식(속도 상한, 벽 통과 금지, 상태 전이 규칙 등)만 회귀 테스트로 굳힌다. 튜닝 값 자체를 assert하지 마라. 이유: 정답을 모르는 값을 박제하면 튜닝할 때마다 깨지는 테스트만 남는다
  - 동시성·네트워크: 단위 테스트 대신 봇 시나리오 + TSan + 지표로 검증한다
  - 일부러 단순하게 만들어 겪어 보는 실험 단계: 검증은 테스트가 아니라 `docs/measurements.md` 기록이다
- 커밋 메시지는 conventional commits 형식을 따를 것 (feat:, fix:, docs:, refactor:, test:, chore:). 설명은 한국어로 쓴다

## 명령어
cmake -S . -B build && cmake --build build                    # 빌드
ctest --test-dir build --output-on-failure                     # 테스트
cmake -S . -B build/tsan -DT1_SANITIZE=thread && cmake --build build/tsan   # TSan 빌드
./build/gameServer/GameServer                                  # 서버 실행 (포트 7777)
