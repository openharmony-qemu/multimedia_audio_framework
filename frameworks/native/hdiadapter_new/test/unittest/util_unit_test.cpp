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
#include "util/audio_hdi_version_manager.h"
#include "audio_stream_enum.h"

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

class SourceWrapperMetadataCallbackTest : public IAudioSourceCallback {
public:
    void OnHdiCaptureCallback(CaptureCallbackType type, const std::vector<uint8_t> &data) override
    {
        callbackCount_++;
        lastType_ = type;
        lastData_ = data;
    }

    int32_t callbackCount_ = 0;
    CaptureCallbackType lastType_ = CB_CAMCORDER_METADATA;
    std::vector<uint8_t> lastData_;
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
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_003
 * @tc.desc   : Test IdHandler action
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_003, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    std::string pipeName = "test";

    SolePipe::SetSolePipeSourceInfo(SOURCE_TYPE_INVALID, AUDIO_INPUT_FLAG_AI, pipeName);
    uint32_t id = idHandler.GetCaptureIdByDeviceClass("primary", SOURCE_TYPE_INVALID,
        HDI_ID_INFO_DEFAULT, AUDIO_INPUT_FLAG_AI);
    EXPECT_EQ(id, idHandler.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_AI, HDI_ID_INFO_DEFAULT));
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_004
 * @tc.desc   : Test IdHandler action
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_004, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    std::string pipeName = "test";

    SolePipe::SetSolePipeSourceInfo(SOURCE_TYPE_INVALID, AUDIO_INPUT_FLAG_UNPROCESS, pipeName);
    uint32_t id = idHandler.GetCaptureIdByDeviceClass("primary", SOURCE_TYPE_INVALID,
        HDI_ID_INFO_DEFAULT, AUDIO_INPUT_FLAG_UNPROCESS);
    EXPECT_EQ(id, idHandler.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_UNPROCESS));
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_005
 * @tc.desc   : Test IdHandler action
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_005, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    std::string pipeName = "test";

    SolePipe::SetSolePipeSourceInfo(SOURCE_TYPE_INVALID, AUDIO_INPUT_FLAG_NORMAL, pipeName);
    uint32_t id = idHandler.GetCaptureIdByDeviceClass("primary", SOURCE_TYPE_INVALID,
        HDI_ID_INFO_DEFAULT, AUDIO_INPUT_FLAG_NORMAL);
    EXPECT_EQ(id, idHandler.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT));
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_006
 * @tc.desc   : Test IdHandler action
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_006, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    std::string pipeName = "test";

    uint32_t id = idHandler.GetCaptureIdByDeviceClass("primary", SOURCE_TYPE_VOICE_CALL,
        HDI_ID_INFO_DEFAULT, AUDIO_INPUT_FLAG_NORMAL);
    EXPECT_EQ(id, idHandler.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT));
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_007
 * @tc.desc   : Test fast primary capture id selection action
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_007, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();

    uint32_t id = idHandler.GetCaptureIdByDeviceClass("primary", SOURCE_TYPE_MIC,
        HDI_ID_INFO_DEFAULT, AUDIO_INPUT_FLAG_FAST);
    EXPECT_EQ(id, idHandler.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FAST, HDI_ID_INFO_DEFAULT));
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_008
 * @tc.desc   : Test fast voip capture id selection action
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_008, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();

    uint32_t id = idHandler.GetCaptureIdByDeviceClass("primary", SOURCE_TYPE_VOICE_COMMUNICATION,
        HDI_ID_INFO_DEFAULT, AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP);
    EXPECT_EQ(id, idHandler.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FAST, HDI_ID_INFO_VOIP));
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_009
 * @tc.desc   : Test fast usb capture id selection action
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_009, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();

    uint32_t id = idHandler.GetCaptureIdByDeviceClass("usb", SOURCE_TYPE_MIC,
        HDI_ID_INFO_DEFAULT, AUDIO_INPUT_FLAG_FAST);
    EXPECT_EQ(id, idHandler.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FAST, HDI_ID_INFO_USB));
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
 * @tc.name   : Test CallbackWrapper API
 * @tc.number : CallbackWrapperUnitTest_003
 * @tc.desc   : Test SourceCallbackWrapper metadata callback dispatch
 */
HWTEST_F(UtilUnitTest, CallbackWrapperUnitTest_003, TestSize.Level1)
{
    SourceCallbackWrapper sourceCbWrapper;
    auto sourceCb = std::make_shared<SourceWrapperMetadataCallbackTest>();
    SourceWrapperMetadataCallbackTest sourceRawCb;
    std::shared_ptr<IAudioSourceCallback> nullSourceCb = nullptr;
    IAudioSourceCallback *nullSourceRawCb = nullptr;
    std::vector<uint8_t> metadata = { 1, 2, 3, 4 };

    sourceCbWrapper.RegistCallback(HDI_CB_CAPTURE_STATE, nullSourceCb);
    sourceCbWrapper.RegistCallback(HDI_CB_CAPTURE_STATE, nullSourceRawCb);
    sourceCbWrapper.RegistCallback(HDI_CB_CAPTURE_CAMCORDER_METADATA, sourceCb);
    sourceCbWrapper.RegistCallback(HDI_CB_CAPTURE_CAMCORDER_METADATA, &sourceRawCb);
    sourceCbWrapper.OnHdiCaptureCallback(CB_CAMCORDER_METADATA, metadata);

    EXPECT_EQ(sourceCb->callbackCount_, 1);
    EXPECT_EQ(sourceCb->lastType_, CB_CAMCORDER_METADATA);
    EXPECT_EQ(sourceCb->lastData_, metadata);
    EXPECT_EQ(sourceRawCb.callbackCount_, 1);
    EXPECT_EQ(sourceRawCb.lastType_, CB_CAMCORDER_METADATA);
    EXPECT_EQ(sourceRawCb.lastData_, metadata);
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

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_016
 * @tc.desc   : Test IdHandler GetRenderIdByDeviceClass with 3da bus keyword (3DA direct)
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_016, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    uint32_t id = idHandler.GetRenderIdByDeviceClass("bus10_3da_out", HDI_ID_INFO_3DA);
    EXPECT_NE(id, HDI_INVALID_ID);

    uint32_t base = idHandler.ParseBase(id);
    EXPECT_EQ(base, HDI_ID_BASE_RENDER);

    uint32_t type = idHandler.ParseType(id);
    EXPECT_EQ(type, HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT);

    std::string info = idHandler.ParseInfo(id);
    EXPECT_EQ(info, HDI_ID_INFO_3DA);

    // keyword match: any bus number / mixport_ prefix converges to the same 3DA direct id
    uint32_t futureBusId = idHandler.GetRenderIdByDeviceClass("mixport_bus11_3da_out", HDI_ID_INFO_3DA);
    EXPECT_EQ(futureBusId, id);
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_017
 * @tc.desc   : Test SelectIdInfo priority: macAddress > deviceNetId > default
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_017, TestSize.Level1)
{
    const std::string mac = "11:22:33:44:55:66";
    const std::string netId = "LocalDevice";

    EXPECT_EQ(IdHandler::SelectIdInfo(mac, netId), mac);
    EXPECT_EQ(IdHandler::SelectIdInfo("", netId), netId);
    EXPECT_EQ(IdHandler::SelectIdInfo("", ""), HDI_ID_INFO_DEFAULT);
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_010
 * @tc.desc   : Test primary_mmap_voip capture id selection for VOIP low latency path
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_010, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();

    uint32_t id = idHandler.GetCaptureIdByDeviceClass("primary_mmap_voip", SOURCE_TYPE_MIC);
    EXPECT_EQ(id, idHandler.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FAST, HDI_ID_INFO_VOIP));
    
    uint32_t base = idHandler.ParseBase(id);
    EXPECT_EQ(base, HDI_ID_BASE_CAPTURE);
    
    uint32_t type = idHandler.ParseType(id);
    EXPECT_EQ(type, HDI_ID_TYPE_FAST);
    
    std::string info = idHandler.ParseInfo(id);
    EXPECT_EQ(info, "voip");
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_011
 * @tc.desc   : Test primary_mmap capture id selection for normal low latency path
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_011, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();

    uint32_t id = idHandler.GetCaptureIdByDeviceClass("primary_mmap", SOURCE_TYPE_MIC);
    EXPECT_EQ(id, idHandler.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FAST, HDI_ID_INFO_DEFAULT));
    
    uint32_t base = idHandler.ParseBase(id);
    EXPECT_EQ(base, HDI_ID_BASE_CAPTURE);
    
    uint32_t type = idHandler.ParseType(id);
    EXPECT_EQ(type, HDI_ID_TYPE_FAST);
    
    std::string info = idHandler.ParseInfo(id);
    EXPECT_EQ(info, "default");
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_012
 * @tc.desc   : Test IdHandler action
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_012, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    std::string pipeName = "test";

    SolePipe::SetSolePipeSourceInfo(SOURCE_TYPE_INVALID, AUDIO_INPUT_FLAG_LIVE, pipeName);
    uint32_t id = idHandler.GetCaptureIdByDeviceClass("primary", SOURCE_TYPE_INVALID,
        HDI_ID_INFO_DEFAULT, AUDIO_INPUT_FLAG_LIVE);
    EXPECT_EQ(id, idHandler.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_LIVE));
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_013
 * @tc.desc   : Test camcorder sole pipe capture id selection
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_013, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    std::string pipeName = "primary_input_camcorder";

    SolePipe::SetSolePipeSourceInfo(SOURCE_TYPE_CAMCORDER, AUDIO_INPUT_FLAG_CAMCORDER, pipeName);
    uint32_t id = idHandler.GetCaptureIdByDeviceClass("primary", SOURCE_TYPE_CAMCORDER,
        HDI_ID_INFO_DEFAULT, AUDIO_INPUT_FLAG_CAMCORDER);
    EXPECT_EQ(id, idHandler.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_CAMCORDER));

    EXPECT_EQ(idHandler.ParseBase(id), HDI_ID_BASE_CAPTURE);
    EXPECT_EQ(idHandler.ParseType(id), HDI_ID_TYPE_PRIMARY);
    EXPECT_EQ(idHandler.ParseInfo(id), HDI_ID_INFO_CAMCORDER);
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_014
 * @tc.desc   : Test primary_extra device class
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_014, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    uint32_t typecId = idHandler.GetRenderIdByDeviceClass("primary_extra", "Typec_Speaker");
    EXPECT_NE(typecId, HDI_INVALID_ID);

    uint32_t btId = idHandler.GetRenderIdByDeviceClass("primary_extra", "BT_Speaker");
    EXPECT_NE(btId, HDI_INVALID_ID);

    uint32_t base = idHandler.ParseBase(typecId);
    EXPECT_EQ(base, HDI_ID_BASE_RENDER);

    uint32_t type = idHandler.ParseType(typecId);
    EXPECT_EQ(type, HDI_ID_TYPE_PRIMARY);

    std::string info = idHandler.ParseInfo(typecId);
    EXPECT_EQ(info, "Typec_Speaker");

    info = idHandler.ParseInfo(btId);
    EXPECT_EQ(info, "BT_Speaker");
}

/**
 * @tc.name   : Test IdHandler API
 * @tc.number : IdHandlerUnitTest_015
 * @tc.desc   : Test IdHandler GetRenderIdByDeviceClass with remote_multichannel
 */
HWTEST_F(UtilUnitTest, IdHandlerUnitTest_015, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    uint32_t id = idHandler.GetRenderIdByDeviceClass("remote_multichannel");
    EXPECT_NE(id, HDI_INVALID_ID);
    
    uint32_t base = idHandler.ParseBase(id);
    EXPECT_EQ(base, HDI_ID_BASE_RENDER);
    
    uint32_t type = idHandler.ParseType(id);
    EXPECT_EQ(type, HDI_ID_TYPE_REMOTE_MULTICHANNEL);
}

/**
 * @tc.name   : Test AudioHdiVersion API
 * @tc.number : AudioHdiVersionManagerUnitTest_001
 * @tc.desc   : IsV61OrAbove with version below v6.0
 */
HWTEST_F(UtilUnitTest, AudioHdiVersionManagerUnitTest_001, TestSize.Level1)
{
    AudioHdiVersionManager::GetInstance().SetVersion(0, 0);
    EXPECT_FALSE(AudioHdiVersionManager::GetInstance().IsV61OrAbove());
}

/**
 * @tc.name   : Test AudioHdiVersion API
 * @tc.number : AudioHdiVersionManagerUnitTest_002
 * @tc.desc   : IsV61OrAbove with version v6.0 (minor below 1)
 */
HWTEST_F(UtilUnitTest, AudioHdiVersionManagerUnitTest_002, TestSize.Level1)
{
    AudioHdiVersionManager::GetInstance().SetVersion(6, 0);
    EXPECT_FALSE(AudioHdiVersionManager::GetInstance().IsV61OrAbove());
}

/**
 * @tc.name   : Test AudioHdiVersion API
 * @tc.number : AudioHdiVersionManagerUnitTest_003
 * @tc.desc   : IsV61OrAbove with version exactly v6.1
 */
HWTEST_F(UtilUnitTest, AudioHdiVersionManagerUnitTest_003, TestSize.Level1)
{
    AudioHdiVersionManager::GetInstance().SetVersion(6, 1);
    EXPECT_TRUE(AudioHdiVersionManager::GetInstance().IsV61OrAbove());
}

/**
 * @tc.name   : Test AudioHdiVersion API
 * @tc.number : AudioHdiVersionManagerUnitTest_004
 * @tc.desc   : IsV61OrAbove with version v6.2
 */
HWTEST_F(UtilUnitTest, AudioHdiVersionManagerUnitTest_004, TestSize.Level1)
{
    AudioHdiVersionManager::GetInstance().SetVersion(6, 2);
    EXPECT_TRUE(AudioHdiVersionManager::GetInstance().IsV61OrAbove());
}

/**
 * @tc.name   : Test AudioHdiVersion API
 * @tc.number : AudioHdiVersionManagerUnitTest_005
 * @tc.desc   : IsV61OrAbove with major version above 6
 */
HWTEST_F(UtilUnitTest, AudioHdiVersionManagerUnitTest_005, TestSize.Level1)
{
    AudioHdiVersionManager::GetInstance().SetVersion(7, 0);
    EXPECT_TRUE(AudioHdiVersionManager::GetInstance().IsV61OrAbove());
}

/**
 * @tc.name   : Test AudioHdiVersion API
 * @tc.number : AudioHdiVersionManagerUnitTest_006
 * @tc.desc   : IsV61OrAbove with major version 5 then restore default state
 */
HWTEST_F(UtilUnitTest, AudioHdiVersionManagerUnitTest_006, TestSize.Level1)
{
    AudioHdiVersionManager::GetInstance().SetVersion(5, 9);
    EXPECT_FALSE(AudioHdiVersionManager::GetInstance().IsV61OrAbove());

    AudioHdiVersionManager::GetInstance().SetVersion(0, 0);
    EXPECT_FALSE(AudioHdiVersionManager::GetInstance().IsV61OrAbove());
}

} // namespace AudioStandard
} // namespace OHOS
