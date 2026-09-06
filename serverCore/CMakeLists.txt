cmake_minimum_required(VERSION 3.10)
project(serverCore CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SERVERCORE_SOURCES
    Acceptor.cpp
    CoreTLS.cpp
    CoreManager.cpp
    IoContext.cpp
    JobSerializer.cpp
    RecvBuffer.cpp
    Session.cpp
    SessionContainer.cpp
    ThreadPool.cpp
    TimeUtils.cpp
)

set(SERVERCORE_HEADERS
    Acceptor.h
    BoostTypes.h
    CoreMacro.h
    CoreTLS.h
    CoreManager.h
    IoContext.h
    Job.h
    JobSerializer.h
    RecvBuffer.h
    SendBuffer.h
    Session.h
    SessionContainer.h
    ThreadPool.h
    TimeUtils.h
    Types.h
)

add_library(serverCore STATIC
    ${SERVERCORE_SOURCES}
    ${SERVERCORE_HEADERS}
)

target_include_directories(serverCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(Boost REQUIRED)
target_include_directories(serverCore PUBLIC ${Boost_INCLUDE_DIRS})

find_package(Threads REQUIRED)
target_link_libraries(serverCore PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(serverCore PRIVATE /W4)
else()
    target_compile_options(serverCore PRIVATE -Wall -Wextra)
endif()
