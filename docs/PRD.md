# PRD: T1 액션 MMORPG 서버

## 목표
float 좌표 기반 액션 MMORPG 서버를 C++17 + Boost.Asio로 직접 만들면서 서버 기술을 학습하고, 측정 수치로 뒷받침되는 면접 포트폴리오를 남긴다.

- 출발점: 예전에 만든 리니지라이크 2D 셀·클릭 이동 MMORPG(`~/Workspace/Unity_Projects/MMORPG`, C# 서버)를 액션 MMO로 다시 만든다.
- **학습 방식**: 처음부터 완성형을 넣지 않는다. 문제를 먼저 겪어 보고, 수치로 남긴 다음, 다음 기법을 도입한다.
- **면접 주장 원칙**: 어필 문구에는 직접 측정한 수치나 코드 근거가 있는 것만 남긴다.

## 사용자
- **개발자 본인**: 학습자. 학습 대상 파트를 직접 구현한다(아래 "작업 분담").
- **면접관**: 결과물(측정 수치, 설계 근거, 코드)을 보는 사람.
- **플레이어(가상)**: 목표 규모 500 CCU, 존당 200명, AOI 안 최대 약 100명. PvE만 한다.

## 핵심 기능 (로드맵)

진행 순서: 기반 + 틱 + 부하 도구 → 이동(0~5단계) → proto 자동화 → AOI → 존·채널 → 몬스터 + A*/LOS → 스킬 판정 → 그룹/보스 AI → 로비/DB → 아이템 → 성장

작업 단위까지 짠 상세 계획은 R1과 R2의 0~2단계뿐이다(아래 "현재 작업 목록"). 나머지 단계는 시작할 때 grilling으로 정한다.

### R1. 기반
"현재 작업 목록"의 1단계 표 참고.

### R2. 이동 0~5단계 (어필 #3)
- 0~2단계: "현재 작업 목록"의 이동 표 참고
- 3단계: 예측만 하고 보정은 하지 않는다. 오차가 시간에 따라 어떻게 커지는지 측정한다
- 4단계: seq/ack + reconciliation(의사 변경 방식). 보정 오차의 분포를 측정한다
- 5단계: 고정 주기 입력 스트림 + 서버 입력 버퍼. 4단계 결과를 보고 할지 말지 정한다
- 순서(2026-10-21 변경): M1(지연 시뮬레이터, 예측 전 기준 수치) → 3단계 → 4단계 → 5단계 판단 → M2(원격 보간) → 충돌. 클라 예측을 먼저 하고, 충돌은 실제 맵과 충돌체를 정한 뒤로 미룬다. 그때는 클라 예측과 4단계 재시뮬레이션에도 서버와 같은 충돌을 넣어야 한다
- 완료 기준: 지연 100±20ms에서 내 캐릭터가 즉시 반응하고, 단계별 수치가 기록돼 있다

### R3. proto 파이프라인 자동화 (이동이 끝난 뒤)
- `GenProto.sh`가 C++ 출력을 이 저장소에 바로 쓴다. PacketGenerator가 C++ ID enum과 핸들러 등록 코드를 생성한다.
- 그 전까지는 proto 메시지 순서를 바꾸지 않고, `PacketHandler.h`의 ID를 손으로 맞춘다.

### R4. AOI (어필 #2)
- 포함: 셀 공간 인덱스, 반경 + 히스테리시스 가시성, spawn/despawn diff, 스냅샷을 보이는 대상에게만 전송, 큰 맵(T-4)
- 정할 것: 심리스 여부(맵 하나를 데이터로만 존 분할 vs 맵 자체를 분리, R5 존 이동 방식과 함께 정한다), R, 셀 크기, margin과 갱신 주기, 스냅샷 배치 패킷화, 원거리 송신 빈도(LOD)
- 완료 기준: R1 기준선과 AOI 적용 후 곡선 비교, 500 CCU 측정

### R5. 존·채널 이동
- 포함: 채널 분리(존당 200명 상한), 포털을 통한 존 이동, `GameLoop::MoveWorld` 경쟁 해결(새 월드 입장이 옛 월드 퇴장보다 먼저 끝날 수 있다)
- 정할 것: 이동 중에 도착한 패킷을 넘기는 순서, 채널 배정 정책

### R6. 몬스터 + 길찾기 (어필 #5)
- 포함: 스폰 풀과 리스폰, 기본 FSM(Idle/Patrol/Chase/Attack/Return/Dead), 정식 A*(G+H; 참조 프로젝트는 greedy best-first였다), LOS 스무딩, 실수 좌표 시작/끝 처리, 이동 검증에 LOS 재사용, 존 로컬 타이머 큐 구현, 몬스터 이동의 틱 처리와 스냅샷 보간, 데이터 테이블 파이프라인
- 정할 것: clearance와 셀 0.5 전환, 경로 재계산 빈도, A* 노드 상한, 몬스터끼리 겹침 처리, 데이터 포맷(SO → JSON 또는 CSV)
- 반복하지 말 참조 버그: 후보마다 FindPath 호출, 스폰 좌표 x/y 오타, WorldToCell 무한 재귀
- 완료 기준: 몬스터가 추적 후 복귀, A*/LOS 테스트 통과, 틱 비용 측정

### R7. 스킬 판정 + 최소 스탯 (어필 #3·#4)
- 포함: 스탯(HP/공격/방어/이속), 스킬 데이터(쿨다운, 시전, 판정 도형, 판정 윈도우), 부채꼴/직선/원 AOE(바닥 도형 × 높이 범위), N틱 판정 윈도우와 다단 히트, 관대화 정책, 투사체 틱 이동, 피격/사망/리스폰, 이펙트(버프/디버프/스턴), 점프·띄우기로 z축 회피
- 정해진 것: PvE이므로 공격자에게 유리하게 판정한다. 풀 리와인드는 하지 않는다. 1틱 = 50ms
- 정할 것: 스킬 입력 예측(애니메이션 즉시 재생 vs 서버 확인 후 재생), RTT 분포로부터 관대화 수치를 역산하는 방법, 텔레그래프 시간과 회피 기준, 데미지 공식, 이펙트 구조
- 반복하지 말 참조 버그: 데미지 이중 차감, 서버 쿨다운 미적용, MP를 MaxHp로 clamp, 투사체 조건 반전
- 완료 기준: 도형 판정 테스트 통과, 지연 100ms에서 회피와 피격이 체감되는지 확인, 판정 비용 측정

### R8. 그룹 AI / 보스 AI (어필 #6)
- 포함: 공통 AI 베이스, 그룹 AI 매니저(공격 슬롯, 진형, 어그로 공유), 보스 컨트롤러(HP 페이즈, 패턴 시퀀스, 텔레그래프), 전투 카메라 자동 전환
- 정할 것: FSM vs Behavior Tree, 슬롯 수와 배분 규칙, 페이즈 전환 동기화, 카메라 전환 조건

### R9. 로비 / DB
- 포함: 계정·캐릭터 CRUD(지금의 가짜 `C_FindMyPlayers` 대체), DB 전용 실행 경로(결과는 존 JobSerializer로 돌려준다), 저장 시점
- 정할 것: DB 종류와 C++ 라이브러리, 스키마, 인증, 저장 주기
- 반복하지 말 것: IO 스레드나 게임 스레드에서 DB를 동기 호출

### R10. 아이템 / 인벤토리
- 포함: 드롭, 칸 인벤토리, 장비/소비 아이템, 장착 시 스탯·이펙트 반영, DB 영속화
- 반복하지 말 참조 버그: `MakeItem`의 null 조건 반전

### R11. 성장
- 포함: 경험치, 레벨업, 레벨별 스탯 테이블, 재화

## 현재 작업 목록 (R1 + R2 이동 + 맵 트랙)

### 작업 분담
- **사용자 (학습 대상)**: 프레이밍 수정, 핸들러 방어(테스트 포함), close 경쟁, 데이터 레이스, JobSerializer 수정, 고정 틱, 이동(서버 + 클라)
- **Claude (인프라)**: 테스트/TSan 빌드, 서버 지표, DummyClient 봇과 부하 측정 스크립트(2026-10-20 사용자 요청으로 이관), 접속/종료 반복 봇(ChurnClient, 1-4 검증), strand 벤치, 클라 지연 시뮬레이터, CC0 에셋 후보 조사, 충돌 굽기 툴, 맵 확장 스크립트
- **사용자 (맵)**: 에셋 최종 선택
- **사용자 파트 진행 방식**: Claude가 과제를 설명하고, 규칙 로직이면 **실패하는 테스트를 먼저** 쓴다 → 사용자가 구현한다 → Claude가 리뷰한다. 담당이 "사용자 (테스트 포함)"이면 Claude는 테스트 케이스 목록만 주고 테스트도 사용자가 쓴다. 감각·튜닝, 동시성, 실험 작업의 검증 방식은 ADR-016을 따른다.

### 1단계: 기반

| # | 담당 | 상태 | 작업 | 검증 |
|---|---|---|---|---|
| 1-1 | Claude | 완료 | GoogleTest(FetchContent)와 `tests/` 타깃 추가, CMake 옵션 `T1_SANITIZE=thread` 추가 | `ctest`로 smoke 테스트 통과, TSan 빌드로 GameServer 실행 |
| 1-2 | Claude 테스트 → 사용자 | 완료 | **recv 프레이밍.** `Session::Read(recvSize)`가 남아 있던 데이터를 빼고 새로 받은 바이트만 기준으로 파싱한다. `header->size < 4`이면 무한 루프에 빠진다(`serverCore/Session.cpp:142-164`). 파싱 부분을 `ParsePackets(RecvBuffer&, onPacket)`(`serverCore/PacketFraming.h`)로 분리한다. 받는 쪽이 크기를 고르지 않도록 `RecvBuffer`를 통째로 받는다. 최대 패킷 크기 상한은 두지 않는다(`uint16`이라 버퍼가 넘치지 않고, `Session`을 봇과 같이 써서 상한을 두면 세션별 설정이 필요하다) | `tests/PacketFramingTest.cpp`의 쪼개진 패킷, 한 번에 여러 패킷, size<4 에러, 헤더 쪼개짐 — 테스트 4개 통과 |
| 1-3 | 사용자 (테스트 포함) | 완료 | **핸들러 방어.** 미등록 id가 오면 `bad_function_call`이 난다. 배열 크기가 `UINT16_MAX`라 마지막 id에서 범위를 벗어난다(`gameServer/PacketHandler.h:10,69-74`). `OnRead`가 반환값을 무시한다(`GameSession.cpp:11`). 결과: 핸들러가 false를 반환하면 연결을 끊는다. `OnRead`가 bool을 반환하고 `ParsePackets`가 false에서 멈춰, 끊은 뒤 같은 recv의 나머지 패킷을 처리하지 않는다. player/world가 아직 없는 패킷은 정상 클라이언트도 타이밍상 보낼 수 있어 처리하지 않고 true를 반환한다(구별은 1-5). 테스트가 PacketHandler를 링크하도록 gameServer를 `gameServerLib`로 분리했다 | `tests/PacketHandlerTest.cpp`: 미등록 id, id `UINT16_MAX`, 등록된 id인데 body를 파싱할 수 없으면 false. `tests/PacketFramingTest.cpp`: 콜백이 false를 반환하면 뒤 패킷을 넘기지 않고 false. 서버에 미등록 id 패킷을 보내면 연결이 끊긴다(수동 확인) |
| 1-4 | 사용자 | 완료 | **세션 close 경쟁**(ADR-008: 세션 strand로 확정). 소켓의 executor를 세션 strand로 바꾸고, strand 밖에서 들어오는 `Connect` 본문, `Send`의 쓰기 시작, `Disconnect`의 close를 post로 넘긴다(`Session.cpp`) | `scripts/churntest.sh`(접속/종료 반복, 끊는 경로 3가지, 끝에 대량 종료)에서 Session 관련 TSan 경고 0건, 서버 크래시 없음. `scripts/loadtest.sh` 전후로 틱, 쓰기 1번당 패킷 수, strand 대기시간을 measurements에 기록 |
| 1-5 | 사용자 | 완료 | **게임 상태 데이터 레이스와 상태 규칙**(ADR-021). 문제: 세션 strand가 World 큐가 쓰는 `Actor::_world`를 읽었고(`PacketHandler.cpp`, `OnDisconnected`), World 잡이 세션의 `_selectdPlayer`를 읽었다(`World.cpp:136`). 입장은 strand → GameLoop 큐 → World 큐로 가고 퇴장은 World 큐로 바로 가서, 입장 중에 끊으면 null World에 `LeavePlayer`를 불러 서버가 죽었다(churntest 재현). null 체크만 하면 퇴장을 보내지 않아 유령이 남을 수 있다. 상태에 맞지 않는 패킷은 무시했고, `C_FindMyPlayers`를 반복하면 플레이어가 쌓였다. 결과: strand가 입장을 보내는 순간 `GameSession::_enteredWorld`(세션 메모)에 적고, 이동·나가기·끊기는 그 값을 보고 같은 World 큐에 바로 Post한다. 월드 목록은 접속을 받기 전에 채우고 고정해 strand가 `GameLoop::FindWorld`로 읽는다. World 잡은 세션 대신 인자 `player`를 쓴다. 상태 규칙(목록 없음 → 목록 받음 → 월드 안)은 strand에서 판단하고 위반은 끊는다. 끊기로 한 세션의 패킷은 `OnRead`에서 처리하지 않는다. World 람다의 raw `this`는 월드가 서버 종료까지 살아 있어 R5로 미룬다 | `tests/SessionStateTest.cpp`(정상 흐름 2, 위반 8). `scripts/churntest.sh`(반복 봇에 입장 중 끊기 `ENTER_CLOSE` 포함)에서 서버 크래시 없음, TSan 경고 0건, 끝난 뒤 존 인원 0명(존 지표 `players=`), 정상 봇 끊김 0. Unity 클라로 입장·이동 확인 |
| 1-6 | Claude 테스트 → 사용자 | 완료 | **JobSerializer.** 시간 예산을 잡 단위로 확인(값은 1-11에서 틱 간격과 함께 정하고 그때까지 64ms, ADR-004), 배치 vector 재사용(`serverCore/JobSerializer.cpp:38,58-61`). 템플릿 `PostAfter` 정리는 1-7로 옮겼다. 테스트에서 io_context를 넣을 수 있도록 생성자가 io_context를 받는다 | `tests/JobSerializerTest.cpp`: 단일 스레드에서 A(1ms 잡 100개)와 B(잡 1개)를 넣으면 B가 A의 잡 "예산(ms) + 2"개 이내에 실행된다(예산 값 자체는 assert하지 않는다). 양보 후에도 잡 순서가 유지되고 빠지는 잡이 없다 |
| 1-7 | Claude 테스트 → 사용자 | 완료 | **고정 틱.** 존마다 `steady_timer` + `expires_at` 절대 데드라인, dt=0.05 고정, 생성 직후부터 시작. 데드라인을 넘기면 건너뛰고 횟수를 센다. 틱 스케줄링은 `serverCore/FixedTicker`(잡 큐, 간격, 콜백을 받는다)로 분리해 소켓·World 없이 테스트하고, World가 하나씩 가진다. 틱 콜백은 존의 JobSerializer 위에서 실행한다(game 스레드가 여러 개다). `_isTickRunning`/`StartTick`/`_lastTickTime` 정리(`gameServer/World.cpp:25-47`). 틱이 `steady_timer`로 바뀌면 `PostAfter`를 쓰는 곳을 확인하고, 호출되면 컴파일되지 않는 템플릿 `PostAfter`(`serverCore/JobSerializer.h`)와 함께 정리한다 | `tests/FixedTickerTest.cpp`: 1초에 20±1틱, 틱마다 10ms 일을 해도 20±1틱(절대 데드라인), 한 틱을 120ms 지연시키면 miss 2회이고 틱 사이 간격이 25ms 이상(몰아서 실행하지 않는다) |
| 1-8 | Claude | 완료 | **서버 지표.** 존별 틱 p50/p99, 데드라인 초과, 잡 대기시간, 세션별 송수신 바이트, CPU를 5초마다 로그로 남긴다 | 봇 50 실행 시 지표가 로그에 찍힌다 |
| 1-9 | Claude (2026-10-20 사용자 요청으로 변경) | 완료 | **DummyClient 봇.** serverCore `Session`을 상속한 BotSession. 봇도 `CoreManager::Init()`(포트 없이)을 쓰도록 사용자가 `Init`/`Start`를 나눴다. FindMyPlayers → EnterWorld(0) → 0.5~2초마다 8방향 랜덤 이동/정지. 멀티스레드 io_context | 봇 200이 모두 `S_EnterWorld`를 받고, 서버 크래시 없음 |
| 1-10 | Claude | | **`JobBench`.** JobSerializer(64ms), JobSerializer(5ms), strand를 비교한다. 처리량, 대기시간 p50/p99, 무거운 큐가 밀려 있을 때 가벼운 큐의 대기시간을 잰다 | `docs/measurements.md`에 결과 표 기록 |
| 1-11 | 함께 | | **AOI 이전 기준선.** 존 하나에 봇 50/100/200을 넣고 틱 시간, 클라당 바이트, CPU를 잰다. 시간 예산 1/5/10/64ms도 비교한다 | N² 곡선과 예산 비교 표 기록 |

### 이동 (위에서부터 진행, 순서는 R2 참고)

| # | 담당 | 상태 | 작업 | 검증 |
|---|---|---|---|---|
| M1-b | Claude |  | **클라 지연 시뮬레이터.** `NetworkManager`에 송수신 지연(평균 + 지터) 옵션 추가 | 지연 100ms 설정 시 RTT 약 200ms |
| M1-c | 함께 |  | 입력부터 반응까지의 지연을 지연 0/50/100ms 조건에서 측정. Nagle 켬/끔(`TCP_NODELAY`)도 비교한다. 지금은 서버와 클라 모두 설정하지 않아서 Nagle이 켜져 있다 | measurements 기록 |
| M0-c | 함께 |  | 500ms 틱과 50ms 틱의 반응 차이 측정(M1-c와 같은 측정 방법) | measurements 기록 |
| M3 | 사용자 |  | **클라 예측(보정 없음).** 상세 작업은 시작 전에 grilling으로 정한다. 서버 이동이 `방향 × 속도 × dt`이고 `S_Move.server_time`이 있어서 proto 변경 없이 시작할 수 있다. 예측 끔 모드에서 내 캐릭터는 원격용 100ms 보간 버퍼를 거치지 않고 최신 `S_Move` 위치를 그린다(구 M1-a, 2026-10-21 합침). 지금은 내 캐릭터도 버퍼를 거친다(`SelectedPlayer.cs:22` → `Actor.UpdateMovement`). 멈췄다 출발할 때는 첫 스냅샷이 오래된 정지 스냅샷과 보간돼 버퍼 지연이 거의 붙지 않지만(지연 0에서 반응 17~50ms 측정), 정지·방향 전환에는 100ms가 더해져 끔/켬 비교에서 예측 효과가 과장된다 | 지연 100ms에서 예측 끔/켬 화면 녹화(끔 모드는 버퍼 없이 최신 위치). 반응 지연 전후, 보정 없을 때 오차가 쌓이는 모습을 measurements에 기록 |
| M0-b1 | Claude 테스트 → 사용자 |  | **입력 검증과 이동 적분(충돌 제외).** 방향을 정규화하고 크기를 1 이하로 자른다. NaN은 거부한다. 이동 적분을 순수 함수로 분리한다. `MoveInfo.currentPosition`과 `PositionInfo.position` 이중 상태를 하나로 합친다. 4단계 재시뮬레이션은 클라가 서버와 같은 적분을 돌려야 하므로 M4 전에 한다 | 크기 >1인 입력도 정상 속도로 움직이고, NaN 입력은 거부한다 |
| M4~M5 | | | 3단계 결과를 보고 grilling으로 정한다 | — |
| M2-a | 사용자 |  | **원격 2틱 보간.** 100ms 렌더 지연, 시계 오프셋 EMA, 점 대 점 보간은 동작한다(`Actor.cs:105-182`). 남은 것: 버퍼 고갈 횟수 로그(M2-b용). 외삽(지금은 마지막 점에서 멈춤), `S_Move.yaw` 회전(지금은 이동 방향으로 회전), 축 변환 모으기는 필요할 때 한다 | 지연 100±20ms에서 원격 캐릭터가 부드럽게 움직인다. 버퍼 고갈 횟수를 로그로 남긴다 |
| M2-b | 함께 |  | 스냅샷 지터와 버퍼 고갈 빈도 측정 | measurements 기록 |
| M0-a | Claude 테스트 → 사용자 |  | **MapComponent 복구**(실제 맵과 충돌체를 정한 뒤). export 포맷에서 `xMax`/`yMax`는 포함하지 않는 값인데, 로더는 `+1`을 해서 범위를 벗어나 읽는다(`gameServer/MapComponent.cpp:36-45`). `WorldToCell`의 행 인덱스가 1칸 밀려 있다(`:55-61`). `CanGo`가 충돌을 확인하지 않는다(`:13-20`). `CellIndex`에 return이 없다(`MapComponent.h:20`). CMake에 등록돼 있지 않다. 충돌맵은 Unity `@Resources/Data/MapData` 경로에서 바로 읽는다. `Map01_Collision.txt`의 0/1 의미를 먼저 확인한다(`1` 2109칸, `0` 391칸이고 `0`이 테두리·장애물 모양이라 `1`이 통로로 보이지만, 참조 프로젝트에서는 `1`이 벽이었다) | 경계 셀 변환, 벽/통로 판정, 50×50 로드 테스트 |
| M0-b2 | Claude 테스트 → 사용자 |  | **벽 미끄러짐.** 중심점 기준으로 막히면 축을 나눠 이동해 벽을 따라 미끄러지게 한다(M0-b1의 순수 함수에 넣는다). 클라 예측과 재시뮬레이션에도 같은 충돌을 넣는다 | 벽에 비스듬히 부딪히면 미끄러진다 |

### 맵 트랙 (병렬 진행)

| # | 담당 | 상태 | 작업 | 검증 |
|---|---|---|---|---|
| T-1 | Claude | 완료 | **CC0 에셋 후보 조사.** 결과: `docs/research/asset-candidates.md`. 지금 쓰는 `block-grass-*`는 Kenney Platformer Kit로 확인했다 | 사용자가 하나 선택(캐릭터 결정 대기) |
| T-2 | — | 보류 | 당분간 Map01을 쓴다 | — |
| T-3 | Claude | | **충돌 굽기 툴**(M0 이후, 셀 크기와 부풀리기는 이때 결정). 셀 중심마다 `Physics.CheckBox`로 검사해 `Tilemap_Collision`을 채우고, 기존 `MapEditor.GenerateMap`으로 내보낸다. 맵을 격자 데이터에서 스크립트로 생성(T-4)하면 격자가 곧 충돌 원본이라, 손으로 얹는 장식이 없으면 이 툴은 필요 없을 수 있다. 이 차례에 판단한다 | 시각화해서 장애물과 일치하는지 확인하고, 서버 충돌이 동작한다 |
| T-4 | Claude (AOI 전) | | **맵 확장 스크립트.** 타일과 장애물을 스크립트로 자동 배치해 큰 맵을 만들고 내보낸다. 심리스 여부를 AOI grilling에서 정한 뒤 만든다 | 서버 로드 + 봇 이동 |

### 전체 검증 기준 (R1 + 이동 0~2단계)
1. `cmake -B build && cmake --build build && ctest --test-dir build`가 통과한다(1-2, 1-3, 1-6, 1-7, M0 테스트).
2. TSan 빌드(`-DT1_SANITIZE=thread`)로 GameServer와 봇 200을 띄우고, 입장/이동/종료를 5분 동안 반복한다. 경고가 0건이어야 한다.
3. 봇 50/100/200 기준선과 JobBench 결과가 `docs/measurements.md`에 기록돼 있다.
4. 지연 시뮬레이터를 100±20ms로 켜고, 원격 캐릭터 보간과 벽 미끄러짐을 눈으로 확인한다. 지연 수치도 기록한다.

## MVP 제외 사항
- 파티, 길드, 채팅
- PvP
- 처음부터 완성형 기법(락프리 큐, 결정론 lockstep 등): 문제가 측정되기 전에는 넣지 않는다

## 디자인 (아트·클라이언트)
- 카메라: 쿼터뷰(직교 카메라, 60° 기울기 기준).
- 맵 아트: 무료 CC0 에셋. 기준은 CC0, 평평한 바닥, 쿼터뷰 로우폴리, 캐릭터와 어울리는 스타일(ADR-017).
- 캐릭터: 지금은 `QuarterView 3D Action BE5`(골드메탈, 출처 표기 의무). KayKit Mystery Monthly(CC0)로 바꿀지는 미정이다.
- 맵: 당분간 Map01(50×50, Kenney Platformer Kit 블록). 큰 맵은 스크립트로 자동 배치한다(ADR-018).

## 어필 포인트
- 단계 대응: #1 잡큐 → R1 · #2 AOI → R4 · #3 예측/권위 → R2·R7 · #4 논타겟 판정 → R7 · #5 길찾기 → R6 · #6 AI → R8
- 기존 어필 문구 수정 사항:

| 기존 | 수정 |
|---|---|
| strand는 핸들러 단위 디스패치, 진입 비용 1/100 | strand도 스냅샷 단위로 배치 실행한다. 자체 큐를 택한 이유는 **틱 데드라인 기준 시간 예산**이다(수치는 1-10/1-11 결과로 채운다) |
| Strand 안 쓰고 직접 잡 큐 | 세션 I/O에는 strand, 존 로직에는 JobSerializer를 쓴다(1-4에서 확정, ADR-008) |
| IOCP 완료 통지 vs Asio Proactor | Asio는 Windows에서는 IOCP 위에 구현되고, macOS/Linux에서는 reactor(kqueue/epoll) 위에서 proactor를 흉내 낸다 |
| 락프리 진입/이탈 | 삭제(mutex 유지). lost wakeup 방지는 지금의 Session/JobSerializer 코드를 근거로 든다 |
| 그래픽은 2D지만 판정은 3D | 쿼터뷰 3D, 이동은 타일 평면, 판정은 높이를 포함한 2.5D |
| float 특성상 셀 경계 enter/leave 빈번 | 경계를 왕복하는 이동 때문에 생기는 spawn/despawn 반복을, 가시 반경에 히스테리시스를 걸어 막았다 |
| NavMesh는 오버스펙 | 평평한 2.5D 지형이고, 충돌·A*·LOS·AOI가 같은 그리드를 쓰며, 동적 장애물은 셀만 바꾸면 된다("서버에서 표현할 수 없다"는 근거로 쓰지 않는다) |
