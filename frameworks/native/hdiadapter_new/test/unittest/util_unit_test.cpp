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

#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include "audio_utils.h"
#include "audio_errors.h"
#include "common/hdi_adapter_info.h"
#include "util/id_handler.h"
#include "util/audio_running_lock.h"
#include "util/ring_buffer_handler.h"
#include "util/callback_wrapper.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class UtilUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

/**
 * @tc.name   : Test Util API
 * @tc.number : UtilUnitTest_001
 * @tc.desc   : Test IdHandler action
 */
HWTEST_F(UtilUnitTest, UtilUnitTest_001, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    uint32_t id = idHandler.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT);
    EXPECT_NE(id, HDI_INVALID_ID);

    id = idHandler.GetRenderIdByDeviceClass("");
    EXPECT_EQ(id, HDI_INVALID_ID);

    id = idHandler.GetCaptureIdByDeviceClass("", SOURCE_TYPE_MIC);
    EXPECT_EQ(id, HDI_INVALID_ID);

    idHandler.IncInfoIdUseCount(id);
    idHandler.DecInfoIdUseCount(id);

    auto ret = idHandler.CheckId(id, HDI_ID_BASE_RENDER);
    EXPECT_EQ(ret, false);

    uint32_t base = idHandler.ParseBase(0);
    EXPECT_EQ(base, 0);

    uint32_t type = idHandler.ParseType(0);
    EXPECT_EQ(type, 0);

    std::string info = idHandler.ParseInfo(HDI_INVALID_ID);
    EXPECT_EQ(info, "");
}

/**
 * @tc.name   : Test Util API
 * @tc.number : UtilUnitTest_002
 * @tc.desc   : Test AudioRunningLock action
 */
HWTEST_F(UtilUnitTest, UtilUnitTest_002, TestSize.Level1)
{
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLock> runningLock = std::make_shared<AudioRunningLock>("test");
    ASSERT_NE(runningLock, nullptr);

    auto ret = runningLock->Lock(-1); // -1: test
    EXPECT_EQ(ret, SUCCESS);

    runningLock->UnLock();
    vector<int32_t> vec = { 0 };
    runningLock->UpdateAppsUid(vec.begin(), vec.end());
#endif
}

/**
 * @tc.name   : Test Util API
 * @tc.number : UtilUnitTest_003
 * @tc.desc   : Test RingBufferHandler action
 */
HWTEST_F(UtilUnitTest, UtilUnitTest_003, TestSize.Level1)
{
    std::shared_ptr<RingBufferHandler> handler = std::make_shared<RingBufferHandler>();
    ASSERT_NE(handler, nullptr);

    handler->Init(10, 10, 10, 10, 10); // 10: test

    vector<uint8_t> bufferWrite = { 0 };
    auto ret = handler->WriteDataToRingBuffer(bufferWrite.data(), bufferWrite.size());
    EXPECT_NE(ret, SUCCESS);

    vector<uint8_t> bufferRead = { 0 };
    bufferRead.resize(bufferWrite.size());
    ret = handler->ReadDataFromRingBuffer(bufferRead.data(), bufferRead.size());
    EXPECT_NE(ret, SUCCESS);

    handler->AddWriteIndex();
    handler->AddReadIndex();
}

/**
 * @tc.name   : Test Util API
 * @tc.number : UtilUnitTest_004
 * @tc.desc   : Test CallbackWrapper action
 */
HWTEST_F(UtilUnitTest, UtilUnitTest_004, TestSize.Level1)
{
    SinkCallbackWrapper sinkCbWrapper;

    sinkCbWrapper.RegistCallback(100, nullptr); // 100: test

    auto sinkCb = sinkCbWrapper.GetCallback(100);
    EXPECT_EQ(sinkCb, nullptr);

    auto sinkRawCb = sinkCbWrapper.GetRawCallback(100);
    EXPECT_EQ(sinkRawCb, nullptr);

    SourceCallbackWrapper sourceCbWrapper;

    sourceCbWrapper.RegistCallback(100, nullptr); // 100: test

    auto sourceCb = sourceCbWrapper.GetCallback(100);
    EXPECT_EQ(sourceCb, nullptr);

    auto sourceRawCb = sourceCbWrapper.GetRawCallback(100);
    EXPECT_EQ(sourceRawCb, nullptr);
}

} // namespace AudioStandard
} // namespace OHOS
