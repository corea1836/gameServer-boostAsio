include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

add_executable(T1Tests
    SmokeTest.cpp
)

target_link_libraries(T1Tests PRIVATE
    serverCore
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(T1Tests)
