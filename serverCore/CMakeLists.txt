cmake_minimum_required(VERSION 3.10)
project(serverCore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB CORE_FILES "*.h" "*.cpp")

# main 함수가 있는 파일 제거
list(REMOVE_ITEM CORE_FILES
"${CMAKE_CURRENT_SOURCE_DIR}/GameServer.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/DummyClient.cpp")

# Boost Asio 찾기
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# pthread 링크
find_package(Threads REQUIRED)

# 서버 실행 파일
add_executable(GameServer
    GameServer.cpp
    ${CORE_FILES}
)


target_link_libraries(GameServer Threads::Threads)

# 더미 클라이언트 실행 파일
add_executable(DummyClient
DummyClient.cpp
${CORE_FILES})

target_link_libraries(DummyClient
Threads::Threads)
