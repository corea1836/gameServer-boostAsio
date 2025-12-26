cmake_minimum_required(VERSION 3.10)
project(serverCore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB CORE_FILES "*.h" "*.cpp")

# 실행파일 생성
add_executable(serverCore
            ${CORE_FILES}
)

# pthread 링크
find_package(Threads REQUIRED)
target_link_libraries(serverCore Threads::Threads)
