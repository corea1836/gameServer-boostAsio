cmake_minimum_required(VERSION 3.15)
project(T1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 예: cmake -B build/tsan -DT1_SANITIZE=thread
set(T1_SANITIZE "" CACHE STRING "켤 sanitizer (예: thread)")
if(T1_SANITIZE)
    add_compile_options(-fsanitize=${T1_SANITIZE} -g)
    add_link_options(-fsanitize=${T1_SANITIZE})
endif()

add_subdirectory(serverCore)
add_subdirectory(gameServer)

enable_testing()
add_subdirectory(tests)
