# 아키텍처

## 디렉토리 구조
```
T1/                                  # 이 저장소 (C++ 서버)
├── CMakeLists.txt                   # 루트. T1_SANITIZE 옵션, tests/ 연결
├── serverCore/                      # 네트워크·실행 기반 (정적 라이브러리)
│   ├── CoreManager, Acceptor        # 포트 열기, 세션 생성 콜백
│   ├── IoContext                    # network / game io_context 두 개
│   ├── ThreadPool                   # io_context를 돌리는 스레드들
│   ├── Session, SessionContainer    # TCP 세션, send queue, recv 프레이밍
│   ├── RecvBuffer, SendBuffer, PacketHeader
│   ├── JobSerializer, Job           # 직렬 실행 잡 큐 (존 로직용)
│   ├── Metrics                      # 1-8 지표: 존 틱·잡 대기, 세션 바이트, CPU를 5초마다 로그
│   └── CoreTLS, TimeUtils, Types
├── gameServer/                      # 게임 로직 + 실행 파일
│   ├── GameServer.cpp               # main
│   ├── GameSession                  # Session 상속, 패킷 → 핸들러
│   ├── PacketHandler                # 패킷 ID enum + 핸들러 테이블 (손으로 맞춤)
│   ├── GameLoop                     # 월드 목록, 월드 입장/퇴장/이동
│   ├── World                        # 존 1개 = JobSerializer 1개 + 틱
│   ├── Actor, Character, Player     # 액터 계층
│   ├── MapComponent                 # 충돌 그리드 (아직 CMake 미등록, M0-a)
│   ├── DummyClient, BotSession, BotPacketHandler   # 부하 테스트 봇 (1-9)
│   └── protobuf/                    # 생성된 *.pb.cc / *.pb.h (원본 .proto는 Unity 쪽)
├── tests/                           # GoogleTest 타깃 T1Tests
├── scripts/loadtest.sh              # 부하 측정 (Release 서버 + 봇, 1-11)
├── docs/                            # 하네스 문서 (PRD, ARCHITECTURE, ADR)
│   └── research/                    # 참고 자료 (조사 결과 등)
└── .claude/                         # 하네스 명령(/harness, /review)과 hook 설정

~/Workspace/Unity_Projects/T1/       # 별도 저장소
├── Client/                          # Unity 클라이언트
│   └── Assets/@Resources/Data/MapData/   # 충돌맵 (서버가 이 경로에서 바로 읽는다)
├── Common/Protocol/                 # proto 원본 (Enum, Struct, Protocol)
├── Server/GenProto.sh               # proto 생성 스크립트
└── Tools/PacketGenerator/           # C# 패킷 코드 생성기
```

## 패턴
- **스레드 모델**: io_context가 두 개다. network io_context를 스레드 5개가, game io_context를 스레드 5개가 돌린다(`GameServer.cpp`). 스레드를 존에 고정하지 않고, 공용 game 풀 위에서 존별 JobSerializer가 동적으로 실행된다(ADR-002).
- **존 = JobSerializer 하나**: `World`의 상태(`_players` 등)는 그 존의 JobSerializer 위에서만 바뀐다. 외부에서는 `World::Post`로 잡을 넣는다. `GameLoop`도 자기 JobSerializer로 월드 목록을 관리한다.
- **세션 I/O**: send queue + `_isWriting` 플래그로 쓰기를 한 번에 하나만 진행한다. push와 플래그 해제를 같은 락 안에서 처리해 lost wakeup을 막는다. 세션 I/O 직렬화 방식(strand 도입 여부)은 1-4에서 정한다(ADR-008).
- **고정 틱**: 존마다 20Hz(dt 50ms) 틱. 절대 데드라인으로 예약하고, 늦으면 건너뛴다(ADR-003, 1-7에서 구현).
- **패킷 처리**: `PacketHeader{size, id}` + protobuf 본문. `GPacketHandler[id]` 테이블로 핸들러를 찾는다. ID는 C# 쪽 선언 순서와 손으로 맞춘다(R3에서 자동화).
- **서버 권위**: 클라는 이동 의사(방향)만 보내고, 서버가 적분한 위치를 내려준다(ADR-011).

## 데이터 흐름
```
[입력]
클라 키 입력 → C_Move(이동 의사) ─TCP→ Session 수신(network 스레드, 프레이밍)
  → GPacketHandler[PKT_C_Move] → World::Post(HandleMove) → 존 JobSerializer(game 스레드)에서 이동 의사 반영

[틱, 50ms마다]
World::Tick → UpdatePosition(dt): 이동 적분 + 충돌 검사
  → MulticastMove: 액터별로 S_Move를 1회 직렬화(World.cpp:91-93) → 각 세션 send queue → async_write ─TCP→ 클라

[클라]
S_Move 수신 → 원격 캐릭터: 2틱(100ms) 지연 보간 / 내 캐릭터: 지금은 서버 값을 그대로 재생(예측은 R2 3단계부터)
```

## 상태 관리
- **존 상태**(플레이어 목록, 위치, 이동 의사): 존의 JobSerializer가 단독 소유한다. 다른 스레드는 잡을 post해서만 바꾼다.
- **월드 목록**: `GameLoop`의 JobSerializer가 소유한다.
- **세션 상태**(소켓, 버퍼, send queue): 세션 객체가 소유한다. 존은 세션을 `weak_ptr`로 참조한다.
- **좌표**: 서버는 (x, y)가 바닥 평면, z가 높이(2.5D). 클라의 축 변환은 한 곳에 모은다(ADR-010).
- **충돌 그리드**: 셀 크기 1.0. Unity에서 내보낸 텍스트 파일을 서버가 읽는다(ADR-015).

## 알려진 위반 사항 (현재 코드)
아래는 위 규칙을 아직 지키지 못하는 곳이다. 해당 작업에서 고친다(`docs/PRD.md` 작업 표).
- `Actor::_world`를 네트워크 스레드에서 읽는다(`PacketHandler.cpp:80,104`). `GameSession`의 `_selectdPlayer`/`_players`를 여러 스레드에서 쓴다 → 1-5
- close가 다른 스레드의 `async_write`/`async_read_some` 시작과 겹칠 수 있다 → 1-4
- recv 프레이밍이 남은 데이터를 무시하고, `size < 4`에서 무한 루프 → 1-2
- 미등록 패킷 id에서 `bad_function_call`, 테이블 범위 초과 → 1-3
- JobSerializer에 시간 예산이 잡 단위로 걸려 있지 않다 → 1-6
- 틱이 고정 dt/절대 데드라인이 아니다 → 1-7
- `TCP_NODELAY`가 서버·클라 모두 꺼져 있다(Nagle 켜짐) → M1-c에서 비교 측정
