cmake_minimum_required(VERSION 3.16)
project(ServerCore)

# 2. Boost 찾기
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

# 로그로 경로가 제대로 잡혔는지 확인 (CMake 실행 시 출력됨)
message(STATUS "Boost Include: ${Boost_INCLUDE_DIRS}")

# 3. 소스 파일 등록
file(GLOB_RECURSE CORE_SRCS "*.cpp")
file(GLOB_RECURSE CORE_HDRS "*.h")

# 4. 라이브러리 생성
add_library(ServerCore STATIC ${CORE_SRCS} ${CORE_HDRS})

# 5. [핵심] Include 경로 강제 주입
# 여기서 ${Boost_INCLUDE_DIRS}를 넣어줘야 Types.h에서 파일을 찾습니다.
target_include_directories(ServerCore PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS} 
)

# 6. PCH 설정 (CorePch.h가 있다면)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CorePch.h")
    # PCH를 사용하면 컴파일 속도가 빨라지지만, 경로 설정이 꼬일 수 있으니
    # 지금처럼 에러가 날 때는 일단 주석 처리하고 빌드해보는 것도 방법입니다.
    target_precompile_headers(ServerCore PRIVATE CorePch.h)
endif()

# 7. 라이브러리 링크
target_link_libraries(ServerCore PUBLIC 
    Threads::Threads
    # Boost::system 같은 라이브러리가 필요할 수 있습니다.
    # 일단 헤더만 찾으면 되니까 비워둡니다.
)
