/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <thread>
#include <cinttypes>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "futex_tool.h"
#include "audio_log_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    static constexpr int64_t TIMEOUT_IN_NS = 300000000; // 300ms
    static constexpr int64_t SHORT_TIMEOUT_IN_NS = 30000000; // 30ms
    static constexpr uint32_t CYCLES_TIMES = 500;
} // namespace

class FutexToolUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
 * @tc.name  : Test FutexTool API
 * @tc.type  : FUNC
 * @tc.number: FutexTool_001
 * @tc.desc  : Test FutexTool interface.
 */
HWTEST(FutexToolUnitTest, FutexTool_001, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_READY;
    std::atomic<uint32_t> readIndex = 0;
    std::atomic<uint32_t> writeIndex = 0;

    std::thread threadWrite([&futexVar, &writeIndex] () {
        writeIndex++;
        FutexTool::FutexWake(&futexVar, IS_READY);
    });

    auto ret = FutexTool::FutexWait(&futexVar, TIMEOUT_IN_NS, [&] () {
        return writeIndex > readIndex;
    });

    threadWrite.join();
    EXPECT_EQ(ret, FUTEX_SUCCESS);
}

/**
 * @tc.name  : Test FutexTool API
 * @tc.type  : FUNC
 * @tc.number: FutexTool_002
 * @tc.desc  : Test FutexTool interface.
 */
HWTEST(FutexToolUnitTest, FutexTool_002, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_NOT_READY;
    std::atomic<uint32_t> readIndex = 0;
    std::atomic<uint32_t> writeIndex = 0;

    std::thread threadWrite([&futexVar, &writeIndex] () {
        writeIndex++;
        FutexTool::FutexWake(&futexVar, IS_READY);
    });

    auto ret = FutexTool::FutexWait(&futexVar, TIMEOUT_IN_NS, [&] () {
        return writeIndex > readIndex;
    });

    threadWrite.join();
    EXPECT_EQ(ret, FUTEX_SUCCESS);
}

/**
 * @tc.name  : Test FutexTool API
 * @tc.type  : FUNC
 * @tc.number: FutexTool_003
 * @tc.desc  : Test FutexTool interface.
 */
HWTEST(FutexToolUnitTest, FutexTool_003, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_READY;
    std::atomic<uint32_t> writeIndex = 0;

    std::thread threadWrite([&futexVar, &writeIndex] () {
        while (writeIndex++ < CYCLES_TIMES) {}
        FutexTool::FutexWake(&futexVar, IS_READY);
    });

    auto ret = FutexTool::FutexWait(&futexVar, TIMEOUT_IN_NS, [&] () {
        return writeIndex >= CYCLES_TIMES;
    });

    threadWrite.join();
    EXPECT_EQ(ret, FUTEX_SUCCESS);
}

/**
 * @tc.name  : Test FutexTool API
 * @tc.type  : FUNC
 * @tc.number: FutexTool_004
 * @tc.desc  : Test FutexTool interface.
 */
HWTEST(FutexToolUnitTest, FutexTool_004, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_NOT_READY;
    std::atomic<uint32_t> writeIndex = 0;

    std::thread threadWrite([&futexVar, &writeIndex] () {
        while (writeIndex++ < CYCLES_TIMES) {}
        FutexTool::FutexWake(&futexVar, IS_READY);
    });

    auto ret = FutexTool::FutexWait(&futexVar, TIMEOUT_IN_NS, [&] () {
        return writeIndex >= CYCLES_TIMES;
    });

    threadWrite.join();
    EXPECT_EQ(ret, FUTEX_SUCCESS);
}

/**
 * @tc.name  : Test FutexTool API
 * @tc.type  : FUNC
 * @tc.number: FutexTool_005
 * @tc.desc  : Test FutexTool interface.
 */
HWTEST(FutexToolUnitTest, FutexTool_005, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_READY;
    std::atomic<uint32_t> readIndex = 0;
    std::atomic<uint32_t> writeIndex = 0;

    std::thread threadWrite([&futexVar, &writeIndex, &readIndex] () {
        while (writeIndex++ < CYCLES_TIMES) {
            if (writeIndex >= readIndex) {
                FutexTool::FutexWake(&futexVar, IS_READY);
            }
        }
    });

    while (readIndex++ < CYCLES_TIMES) {
        auto ret = FutexTool::FutexWait(&futexVar, TIMEOUT_IN_NS, [&] () {
            return writeIndex >= readIndex;
        });
        EXPECT_EQ(ret, FUTEX_SUCCESS);
    }

    threadWrite.join();
}

/**
 * @tc.name  : Test FutexTool API
 * @tc.type  : FUNC
 * @tc.number: FutexTool_006
 * @tc.desc  : Test FutexTool interface.
 */
HWTEST(FutexToolUnitTest, FutexTool_006, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_NOT_READY;
    std::atomic<uint32_t> readIndex = 0;
    std::atomic<uint32_t> writeIndex = 0;

    std::thread threadWrite([&futexVar, &writeIndex, &readIndex] () {
        while (writeIndex++ < CYCLES_TIMES) {
            if (writeIndex >= readIndex) {
                FutexTool::FutexWake(&futexVar, IS_READY);
            }
        }
    });

    while (readIndex++ < CYCLES_TIMES) {
        auto ret = FutexTool::FutexWait(&futexVar, TIMEOUT_IN_NS, [&] () {
            return writeIndex >= readIndex;
        });
        EXPECT_EQ(ret, FUTEX_SUCCESS);
    }

    threadWrite.join();
}

/**
 * @tc.name  : Test FutexTool API
 * @tc.type  : FUNC
 * @tc.number: FutexTool_007
 * @tc.desc  : Test FutexTool interface.
 */
HWTEST(FutexToolUnitTest, FutexTool_007, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_READY;

    auto ret = FutexTool::FutexWait(&futexVar, SHORT_TIMEOUT_IN_NS, [] () {
        return false;
    });

    EXPECT_EQ(ret, FUTEX_TIMEOUT);
}

/**
 * @tc.name  : Test FutexTool API
 * @tc.type  : FUNC
 * @tc.number: FutexTool_008
 * @tc.desc  : Test FutexTool interface.
 */
HWTEST(FutexToolUnitTest, FutexTool_008, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar(100);

    auto ret = FutexTool::FutexWait(&futexVar, SHORT_TIMEOUT_IN_NS, [] () {
        return false;
    });

    EXPECT_EQ(ret, FUTEX_INVALID_PARAMS);
}

/**
 * @tc.name  : Test ProcessVolumeData API
 * @tc.type  : FUNC
 * @tc.number: ProcessVolumeData_001
 * @tc.desc  : Test ProcessVolumeData interface.
 */
HWTEST(FutexToolUnitTest, ProcessVolumeData_001, TestSize.Level1)
{
    AudioLogUtils audioLogUtils;
    std::string logTag = "test_log_tag";
    ChannelVolumes vols;
    vols.channel = STEREO;
    vols.volStart[0] = 0;
    vols.volStart[1] = 0;
    int64_t count = 10;
    audioLogUtils.ProcessVolumeData(logTag, vols, count);
    EXPECT_NE(count, 9);
}

/**
 * @tc.name  : Test ProcessVolumeData API
 * @tc.type  : FUNC
 * @tc.number: ProcessVolumeData_002
 * @tc.desc  : Test ProcessVolumeData interface.
 */
HWTEST(FutexToolUnitTest, ProcessVolumeData_002, TestSize.Level1)
{
    AudioLogUtils audioLogUtils;
    std::string logTag = "test_log_tag";
    ChannelVolumes vols;
    vols.channel = STEREO;
    vols.volStart[0] = 1;
    vols.volStart[1] = 1;
    int64_t count = -10;
    audioLogUtils.ProcessVolumeData(logTag, vols, count);
    EXPECT_NE(count, -9);
}

HWTEST(FutexToolUnitTest, FutexWake_001, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_NOT_READY;
    auto ret = FutexTool::FutexWake(&futexVar, IS_READY);
    EXPECT_EQ(ret, FUTEX_SUCCESS);
    EXPECT_EQ(futexVar.load(), IS_READY);
}

HWTEST(FutexToolUnitTest, FutexWake_002, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_READY;
    auto ret = FutexTool::FutexWake(&futexVar, IS_READY);
    EXPECT_EQ(ret, FUTEX_SUCCESS);
}

HWTEST(FutexToolUnitTest, FutexWake_003, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_PRE_EXIT;
    auto ret = FutexTool::FutexWake(&futexVar, IS_READY);
    EXPECT_EQ(ret, FUTEX_SUCCESS);
}

HWTEST(FutexToolUnitTest, FutexWake_004, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar(100);
    auto ret = FutexTool::FutexWake(&futexVar, IS_READY);
    EXPECT_EQ(ret, FUTEX_INVALID_PARAMS);
}

HWTEST(FutexToolUnitTest, FutexWake_005, TestSize.Level1)
{
    auto ret = FutexTool::FutexWake(nullptr, IS_READY);
    EXPECT_EQ(ret, FUTEX_INVALID_PARAMS);
}

HWTEST(FutexToolUnitTest, FutexWake_006, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_NOT_READY;
    std::thread thread([&futexVar] () {
        futexVar.store(IS_READY);
    });
    thread.join();
    auto ret = FutexTool::FutexWake(&futexVar, IS_READY);
    EXPECT_EQ(ret, FUTEX_SUCCESS);
}

HWTEST(FutexToolUnitTest, FutexWake_007, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_NOT_READY;
    auto ret = FutexTool::FutexWake(&futexVar, IS_PRE_EXIT);
    EXPECT_EQ(ret, FUTEX_SUCCESS);
    EXPECT_EQ(futexVar.load(), IS_PRE_EXIT);
}

HWTEST(FutexToolUnitTest, FutexWaitTimeout_001, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_READY;
    auto ret = FutexTool::FutexWait(&futexVar, SHORT_TIMEOUT_IN_NS, [] () {
        return false;
    });
    EXPECT_EQ(ret, FUTEX_TIMEOUT);
}

HWTEST(FutexToolUnitTest, FutexWaitTimeout_002, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_NOT_READY;
    auto ret = FutexTool::FutexWait(&futexVar, SHORT_TIMEOUT_IN_NS, [] () {
        return false;
    });
    EXPECT_EQ(ret, FUTEX_TIMEOUT);
}

HWTEST(FutexToolUnitTest, FutexWaitNegativeTimeout_001, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_READY;
    auto ret = FutexTool::FutexWait(&futexVar, -1, [] () {
        return false;
    });
    EXPECT_NE(ret, FUTEX_SUCCESS);
}

HWTEST(FutexToolUnitTest, FutexWakeDifferentValues_001, TestSize.Level1)
{
    std::atomic<uint32_t> futexVar = IS_NOT_READY;
    auto ret1 = FutexTool::FutexWake(&futexVar, IS_READY);
    EXPECT_EQ(ret1, FUTEX_SUCCESS);
    
    futexVar.store(IS_NOT_READY);
    auto ret2 = FutexTool::FutexWake(&futexVar, IS_NOT_READY);
    EXPECT_EQ(ret2, FUTEX_SUCCESS);
    
    futexVar.store(IS_NOT_READY);
    auto ret3 = FutexTool::FutexWake(&futexVar, IS_PRE_EXIT);
    EXPECT_EQ(ret3, FUTEX_SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS