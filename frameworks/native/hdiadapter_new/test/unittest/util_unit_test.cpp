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
#include "util/kv_pair.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
constexpr uint64_t TEST_MAX_FRAME_NUM = 5;
constexpr uint64_t TEST_READ_START_POSITION = 10;
constexpr uint64_t TEST_MAX_ARRAY_LEN = 100;
class UtilUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_001
 * @tc.desc   : Test IdHandler action
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_001, TestSize.Level1)
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
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_002
 * @tc.desc   : Test IdHandler action
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_002, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    uint32_t id = idHandler.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_HEARING_AID);
    EXPECT_NE(id, HDI_INVALID_ID);

    uint32_t hearingAidId = idHandler.GetRenderIdByDeviceClass("hearing_aid");
    EXPECT_EQ(hearingAidId, id);
}

/**
 * @tc.name   : Test AudioRunningLock API
 * @tc.number : AudioRunningLockUnitTest_001
 * @tc.desc   : Test AudioRunningLock action
 */
HWTEST_F(UtilUnitTest, AudioRunningLockUnitTest_001, TestSize.Level1)
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
 * @tc.name   : Test AudioRunningLock API
 * @tc.number : AudioRunningLockUnitTest_002
 * @tc.desc   : Test AudioRunningLock action
 */
HWTEST_F(UtilUnitTest, AudioRunningLockUnitTest_002, TestSize.Level1)
{
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLock> runningLock = std::make_shared<AudioRunningLock>("test");
    ASSERT_NE(runningLock, nullptr);

    auto ret = runningLock->UpdateAppsUidToPowerMgr();
    EXPECT_EQ(ret, SUCCESS);
    
    vector<int32_t> vec = { 0 };
    runningLock->UpdateAppsUid(vec.begin(), vec.end());

    ret = runningLock->UpdateAppsUidToPowerMgr();
    EXPECT_EQ(ret, SUCCESS);
#endif
}

/**
 * @tc.name   : Test AudioRunningLock API
 * @tc.number : AudioRunningLockUnitTest_003
 * @tc.desc   : Test AudioRunningLock action
 */
HWTEST_F(UtilUnitTest, AudioRunningLockUnitTest_003, TestSize.Level1)
{
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLock> runningLock = std::make_shared<AudioRunningLock>("test");
    ASSERT_NE(runningLock, nullptr);

    runningLock->isLocked_ = true;

    auto ret = runningLock->UpdateAppsUidToPowerMgr();
    EXPECT_EQ(ret, SUCCESS);
    
    vector<int32_t> vec = { 0 };
    runningLock->UpdateAppsUid(vec.begin(), vec.end());

    ret = runningLock->UpdateAppsUidToPowerMgr();
    EXPECT_EQ(ret, SUCCESS);
#endif
}

/**
 * @tc.name   : Test RingBufferHandler API
 * @tc.number : RingBufferHandlerUnitTest_001
 * @tc.desc   : Test RingBufferHandler action
 */
HWTEST_F(UtilUnitTest, RingBufferHandlerUnitTest_001, TestSize.Level1)
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
 * @tc.name   : Test RingBufferHandler API
 * @tc.number : RingBufferHandlerUnitTest_002
 * @tc.desc   : Test RingBufferHandler action
 */
HWTEST_F(UtilUnitTest, RingBufferHandlerUnitTest_002, TestSize.Level1)
{
    auto ringBuffer = std::make_shared<RingBufferHandler>();
    EXPECT_NE(ringBuffer, nullptr);

    const uint32_t dataLen = static_cast<uint32_t>(sizeof(uint32_t));
    const uint32_t maxFrameNum = TEST_MAX_FRAME_NUM;
    const uint32_t arrayLen = TEST_MAX_ARRAY_LEN;
    ringBuffer->Init(arrayLen, 1, dataLen, 1, maxFrameNum);
    std::vector<uint32_t> testNums;
    for (uint32_t i = 0; i < arrayLen; ++i) {
        testNums.emplace_back(i);
    }

    const uint32_t readStart = TEST_READ_START_POSITION;
    for (uint32_t i = 0; i < readStart; ++i) {
        EXPECT_EQ(ringBuffer->WriteDataToRingBuffer(reinterpret_cast<uint8_t *>(&testNums[i]), dataLen), SUCCESS);
    }

    for (uint32_t i = readStart; i < arrayLen; ++i) {
        EXPECT_EQ(ringBuffer->WriteDataToRingBuffer(reinterpret_cast<uint8_t *>(&testNums[i]), dataLen), SUCCESS);
        uint32_t val = 0;
        EXPECT_EQ(ringBuffer->ReadDataFromRingBuffer(reinterpret_cast<uint8_t *>(&val), dataLen), SUCCESS);
        EXPECT_EQ(val, testNums[i - maxFrameNum + 1]);
    }
}

/**
 * @tc.name   : Test CallbackWrapper API
 * @tc.number : CallbackWrapperUnitTest_001
 * @tc.desc   : Test SinkCallbackWrapper action
 */
HWTEST_F(UtilUnitTest, CallbackWrapperUnitTest_001, TestSize.Level1)
{
    SinkCallbackWrapper sinkCbWrapper;

    std::shared_ptr<IAudioSinkCallback> sinkCb = nullptr;
    sinkCbWrapper.RegistCallback(HDI_CB_RENDER_STATE, sinkCb);
    IAudioSinkCallback *sinkRawCb = nullptr;
    sinkCbWrapper.RegistCallback(HDI_CB_RENDER_STATE, sinkRawCb);
    std::function<std::shared_ptr<IAudioSinkCallback>(uint32_t)> cbGenerator = [](uint32_t renderId) {
        return nullptr;
    };
    sinkCbWrapper.RegistCallbackGenerator(HDI_CB_RENDER_STATE, cbGenerator);
    sinkCb = sinkCbWrapper.GetCallback(HDI_CB_RENDER_STATE, HDI_INVALID_ID);
    EXPECT_EQ(sinkCb, nullptr);
    sinkRawCb = sinkCbWrapper.GetRawCallback(HDI_CB_RENDER_STATE);
    EXPECT_EQ(sinkRawCb, nullptr);

    sinkCbWrapper.OnRenderSinkParamChange("", NONE, "", "");
    sinkCbWrapper.OnRenderSinkStateChange(0, false);
}

/**
 * @tc.name   : Test CallbackWrapper API
 * @tc.number : CallbackWrapperUnitTest_002
 * @tc.desc   : Test SourceCallbackWrapper action
 */
HWTEST_F(UtilUnitTest, CallbackWrapperUnitTest_002, TestSize.Level1)
{
    SourceCallbackWrapper sourceCbWrapper;

    std::shared_ptr<IAudioSourceCallback> sourceCb = nullptr;
    sourceCbWrapper.RegistCallback(HDI_CB_CAPTURE_STATE, sourceCb);
    IAudioSourceCallback *sourceRawCb = nullptr;
    sourceCbWrapper.RegistCallback(HDI_CB_CAPTURE_STATE, sourceRawCb);
    std::function<std::shared_ptr<IAudioSourceCallback>(uint32_t)> cbGenerator = [](uint32_t captureId) {
        return nullptr;
    };
    sourceCbWrapper.RegistCallbackGenerator(HDI_CB_CAPTURE_STATE, cbGenerator);
    sourceCb = sourceCbWrapper.GetCallback(HDI_CB_CAPTURE_STATE, HDI_INVALID_ID);
    EXPECT_EQ(sourceCb, nullptr);
    sourceRawCb = sourceCbWrapper.GetRawCallback(HDI_CB_CAPTURE_STATE);
    EXPECT_EQ(sourceRawCb, nullptr);

    sourceCbWrapper.OnCaptureSourceParamChange("", NONE, "", "");
    sourceCbWrapper.OnCaptureState(false);
    sourceCbWrapper.OnWakeupClose();
}

/**
 * @tc.name   : Test KvPair API
 * @tc.number : KvPairUnitTest_001
 * @tc.desc   : Test KvPair action
 */
HWTEST_F(UtilUnitTest, KvPairUnitTest_001, TestSize.Level1)
{
    KvPair<std::string> kv;
    kv.Set<uint32_t>("test", 1);
    uint32_t value = 0;
    auto ret = kv.Get<uint32_t>("test", value);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(value, 1);
    kv.Erase("test");
    ret = kv.Get<uint32_t>("test_1", value);
    EXPECT_NE(ret, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
