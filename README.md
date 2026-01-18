# gameServer-boostAsio
boost Asio 게임 서버 입니다. cpp 를 학습을 함께 진행합니다.

# feature
- Lock 구간을 최소화 하기 위해 게임 서버는 비지니스 로직을 직렬화합니다.(링크 : https://github.com/corea1836/gameServer-boostAsio/pull/11#issuecomment-3742494049)
- 게임 서버는 Job 이 발생했을 때만 처리하도록 최적화 했습니다.(링크 : https://github.com/corea1836/gameServer-boostAsio/pull/11#issuecomment-3757867113)
- Client-Server 통신은 Protobuf 를 사용하며 소켓 I/O 는 Async 하게 작동하도록 최적화 했습니다.(링크 : https://github.com/corea1836/gameServer-boostAsio/pull/9#pullrequestreview-3637335059)

# 고민, 학습한 내용
- Boost Asio 의 동작 원리(Proactor 패턴) 가 어떻게 구현되었는지 분석하고, JobSerialize 와 연동할 수 있도록 고민했습니다.(링크 : https://github.com/corea1836/gameServer-boostAsio/pull/7#issuecomment-3717243960)
- 게임 서버에 적합한 재귀 SpinLock 의 구현 방법을 분석하고 고민했습니다.(링크 : https://github.com/corea1836/gameServer-boostAsio/pull/4#issuecomment-3701283522)
- 가변인자 템플릿을 통해 멤버 함수 포인터를 바인딩 했습니다. 비지니스 로직 구현을 유연하게 할 수 있도록 분석하고 고민했습니다.(링크 : https://github.com/corea1836/gameServer-boostAsio/pull/11#discussion_r2702163602)
- 좋은 c++ 코드 작성을 위해 고민했습니다.
    - lvalue, rvalue 의 Identity(메모리 고유 식별 주소, 식별가능여부) 에 대해 이해했습니다.(링크 : https://github.com/corea1836/gameServer-boostAsio/pull/11#discussion_r2702167287)
    - static_pointer_cast 와 shared_from_this 를 연동하며 상속 관계에서 포인터 타입의 캐스팅에 대해 이해했습니다.(링크 : https://github.com/corea1836/gameServer-boostAsio/pull/11#discussion_r2702212850)
    - OS 에 영향받지 않는 Packet Header 정의를 위해 pcaking 을 이해했습니다.(링크 : https://github.com/corea1836/gameServer-boostAsio/pull/9#discussion_r2674619000)
    - 비트 마스크를 통해 원하는 값을 체크하고 Atomic 한 연산에 사용하는 방법을 이해했습니다.(링크 : https://github.com/corea1836/gameServer-boostAsio/pull/4#discussion_r2660252446)
