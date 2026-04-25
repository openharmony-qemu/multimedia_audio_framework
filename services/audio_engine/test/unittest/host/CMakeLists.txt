cmake_minimum_required(VERSION 3.14)
project(audio_engine_host_tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS CLT: C++ headers are in the SDK but clang doesn't find them automatically
if(APPLE)
    execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    include_directories(BEFORE SYSTEM "${SDK_PATH}/usr/include/c++/v1")
endif()

# Stub include path MUST come before real headers so stubs are found first
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../utils                     # hpae_format_convert.h, hpae_backoff_controller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..                           # audio_engine/ (for node/include/*.h)
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..                      # audio_framework/ root
)

# Find googletest
find_package(GTest REQUIRED)

# ============================================================
# Target: hpae_format_convert_host_test
# ============================================================
add_executable(hpae_format_convert_host_test
    hpae_format_convert_host_test.cpp
    ../../../utils/hpae_format_convert.cpp
)
target_link_libraries(hpae_format_convert_host_test PRIVATE GTest::gtest GTest::gtest_main)

# ============================================================
# Target: hpae_backoff_controller_host_test
# ============================================================
add_executable(hpae_backoff_controller_host_test
    hpae_backoff_controller_host_test.cpp
    ../../../utils/hpae_backoff_controller.cpp
)
target_link_libraries(hpae_backoff_controller_host_test PRIVATE GTest::gtest GTest::gtest_main)

# ============================================================
# CTest integration
# ============================================================
enable_testing()
include(GoogleTest)
gtest_discover_tests(hpae_format_convert_host_test)
gtest_discover_tests(hpae_backoff_controller_host_test)
