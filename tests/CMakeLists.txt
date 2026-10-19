include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

add_executable(T1Tests
    SmokeTest.cpp
    PacketFramingTest.cpp
    JobSerializerTest.cpp
    FixedTickerTest.cpp
)

target_link_libraries(T1Tests PRIVATE
    serverCore
    GTest::gtest_main
)

include(GoogleTest)
# 무한 루프 버그가 멈춤이 아니라 실패로 보이게 한다 (ctest 기본값은 1500초)
gtest_discover_tests(T1Tests PROPERTIES TIMEOUT 10)
