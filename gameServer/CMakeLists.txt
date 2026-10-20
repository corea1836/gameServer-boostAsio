cmake_minimum_required(VERSION 3.10)
project(gameServer CXX)

find_package(Protobuf REQUIRED)

# protoc로 생성한 파일 — import 관계 때문에 셋 다 필요
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protobuf)
set(PROTO_SOURCES
    ${PROTO_DIR}/Enum.pb.cc
    ${PROTO_DIR}/Struct.pb.cc
    ${PROTO_DIR}/Protocol.pb.cc
)

# 두 실행 파일이 공통으로 쓰는 파일 (이제 Protobuf 파일들만 공통입니다)
set(COMMON_SOURCES
    ${PROTO_SOURCES}
)

# --- 서버 로직 (main 제외) — 테스트에서도 링크한다 ---
add_library(gameServerLib STATIC
    Character.cpp
    GameSession.cpp
    PacketHandler.cpp
    Actor.cpp
    GameLoop.cpp
    GameManager.cpp
    Player.cpp
    World.cpp
    ${COMMON_SOURCES}
)

target_include_directories(gameServerLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROTO_DIR}
)

target_link_libraries(gameServerLib PUBLIC
    serverCore
    protobuf::libprotobuf
)

# --- 서버 실행 파일 ---
add_executable(GameServer
    GameServer.cpp
)

target_link_libraries(GameServer PRIVATE
    gameServerLib
)

# --- 부하 테스트 봇 실행 파일 (1-9) ---
add_executable(DummyClient
    DummyClient.cpp
    BotSession.cpp
    BotPacketHandler.cpp
    ${COMMON_SOURCES}
)

target_include_directories(DummyClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROTO_DIR}
)

target_link_libraries(DummyClient PRIVATE
    serverCore
    protobuf::libprotobuf
)
