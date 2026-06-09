/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AIUT_LOG_H
#define AIUT_LOG_H

#include <cstdio>
#include <cstring>
#include "gtest/gtest.h"

namespace AIUT {

// Extract filename from full path (handles both '/' and '\\' separators)
inline const char* GetFileName(const char* file)
{
    if (file == nullptr) {
        return "";
    }
    const char* result = file;
    for (const char* p = file; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') {
            result = p + 1;
        }
    }
    return result;
}

// Get current GTest test case name automatically
inline const char* GetTestName()
{
    auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
    return info ? info->name() : "Unknown";
}

// Get current GTest failure count for change detection
inline int AiutGetFailureCount()
{
    return ::testing::UnitTest::GetInstance()
        ->current_test_info()->result()->total_part_count();
}

// Check if a new failure just occurred since prevFailCount
inline bool AiutDidNewlyFail(int prevFailCount)
{
    return AiutGetFailureCount() > prevFailCount;
}

} // namespace AIUT

/*
 * AIUT logging macros - automatically include current test name and file:line
 *
 * Usage:
 *   AIUT_DEBUG_LOG("entry");                        // debug logging
 *   AIUT_INFO_LOG("entry");                         // test entry point
 *   AIUT_INFO_LOG("step %d completed", step);       // with format args
 *   AIUT_ERROR_LOG("failed with ret=%d", ret);      // error logging
 *   AIUT_WARNING_LOG("unexpected value: %d", val);  // warning logging
 *
 * Output format:
 *   [AIUT DEBUG][TestName][file.cpp:42] entry
 *   [AIUT INFO][TestName][file.cpp:42] entry
 *   [AIUT ERROR][TestName][file.cpp:50] failed with ret=-1
 */

#define AIUT_DEBUG_LOG(fmt, ...) \
    printf("[AIUT DEBUG][%s][%s:%d] " fmt "\n", \
        AIUT::GetTestName(), AIUT::GetFileName(__FILE__), __LINE__, ##__VA_ARGS__)

#define AIUT_INFO_LOG(fmt, ...) \
    printf("[AIUT INFO][%s][%s:%d] " fmt "\n", \
        AIUT::GetTestName(), AIUT::GetFileName(__FILE__), __LINE__, ##__VA_ARGS__)

#define AIUT_ERROR_LOG(fmt, ...) \
    printf("[AIUT ERROR][%s][%s:%d] " fmt "\n", \
        AIUT::GetTestName(), AIUT::GetFileName(__FILE__), __LINE__, ##__VA_ARGS__)

#define AIUT_WARNING_LOG(fmt, ...) \
    printf("[AIUT WARNING][%s][%s:%d] " fmt "\n", \
        AIUT::GetTestName(), AIUT::GetFileName(__FILE__), __LINE__, ##__VA_ARGS__)

// Alias: AIUT_WARNNING_LOG (preserving original spelling preference)
#define AIUT_WARNNING_LOG AIUT_WARNING_LOG

/*
 * AIUT assertion logging macros - combine GTest assertion with AIUT logging.
 * When assertion fails, it prints the test name, file:line, and custom context message.
 *
 * Detection: Records failure count BEFORE the assertion, then compares AFTER.
 * This avoids false positives when a previous assertion already failed.
 *
 * Usage:
 *   AIUT_EXPECT_NE_INFO_LOG(ptr, nullptr, "CreateInstance failed, ptr=%p", ptr);
 *   AIUT_EXPECT_EQ_INFO_LOG(ret, 0, "Call failed, ret=%d", ret);
 *   AIUT_ASSERT_TRUE_INFO_LOG(Init(), "Init failed, errno=%d", errno);
 *   AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "Create returned null, result=%d", result);
 *
 * When assertion FAILS, output (in addition to GTest's own output):
 *   [AIUT EXPECT FAIL][TestName][file.cpp:55] EXPECT_NE(ptr, nullptr) | CreateInstance failed, ptr=0x0
 *   [AIUT ASSERT FAIL][TestName][file.cpp:60] ASSERT_EQ(ret, 0) | Call failed, ret=-1
 *
 * When assertion PASSES, no extra output (only GTest internal tracking).
 */

// ---- EXPECT_EQ with INFO log on failure ----
#define AIUT_EXPECT_EQ_INFO_LOG(val1, val2, fmt, ...) \
    do { \
        int aiut_prev_count_ = AIUT::AiutGetFailureCount(); \
        EXPECT_EQ(val1, val2); \
        if (AIUT::AiutDidNewlyFail(aiut_prev_count_)) { \
            printf("[AIUT EXPECT FAIL][%s][%s:%d] EXPECT_EQ(" #val1 ", " #val2 ") | " fmt "\n", \
                AIUT::GetTestName(), AIUT::GetFileName(__FILE__), __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

// ---- EXPECT_NE with INFO log on failure ----
#define AIUT_EXPECT_NE_INFO_LOG(val1, val2, fmt, ...) \
    do { \
        int aiut_prev_count_ = AIUT::AiutGetFailureCount(); \
        EXPECT_NE(val1, val2); \
        if (AIUT::AiutDidNewlyFail(aiut_prev_count_)) { \
            printf("[AIUT EXPECT FAIL][%s][%s:%d] EXPECT_NE(" #val1 ", " #val2 ") | " fmt "\n", \
                AIUT::GetTestName(), AIUT::GetFileName(__FILE__), __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

// ---- EXPECT_TRUE with INFO log on failure ----
#define AIUT_EXPECT_TRUE_INFO_LOG(expr, fmt, ...) \
    do { \
        int aiut_prev_count_ = AIUT::AiutGetFailureCount(); \
        EXPECT_TRUE(expr); \
        if (AIUT::AiutDidNewlyFail(aiut_prev_count_)) { \
            printf("[AIUT EXPECT FAIL][%s][%s:%d] EXPECT_TRUE(" #expr ") | " fmt "\n", \
                AIUT::GetTestName(), AIUT::GetFileName(__FILE__), __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

// ---- ASSERT_EQ with INFO log on failure ----
#define AIUT_ASSERT_EQ_INFO_LOG(val1, val2, fmt, ...) \
    do { \
        int aiut_prev_count_ = AIUT::AiutGetFailureCount(); \
        ASSERT_EQ(val1, val2); \
        if (AIUT::AiutDidNewlyFail(aiut_prev_count_)) { \
            printf("[AIUT ASSERT FAIL][%s][%s:%d] ASSERT_EQ(" #val1 ", " #val2 ") | " fmt "\n", \
                AIUT::GetTestName(), AIUT::GetFileName(__FILE__), __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

// ---- ASSERT_NE with INFO log on failure ----
#define AIUT_ASSERT_NE_INFO_LOG(val1, val2, fmt, ...) \
    do { \
        int aiut_prev_count_ = AIUT::AiutGetFailureCount(); \
        ASSERT_NE(val1, val2); \
        if (AIUT::AiutDidNewlyFail(aiut_prev_count_)) { \
            printf("[AIUT ASSERT FAIL][%s][%s:%d] ASSERT_NE(" #val1 ", " #val2 ") | " fmt "\n", \
                AIUT::GetTestName(), AIUT::GetFileName(__FILE__), __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

// ---- ASSERT_TRUE with INFO log on failure ----
#define AIUT_ASSERT_TRUE_INFO_LOG(expr, fmt, ...) \
    do { \
        int aiut_prev_count_ = AIUT::AiutGetFailureCount(); \
        ASSERT_TRUE(expr); \
        if (AIUT::AiutDidNewlyFail(aiut_prev_count_)) { \
            printf("[AIUT ASSERT FAIL][%s][%s:%d] ASSERT_TRUE(" #expr ") | " fmt "\n", \
                AIUT::GetTestName(), AIUT::GetFileName(__FILE__), __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

#endif // AIUT_LOG_H
