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

#include "bluetooth_audio_render_sink_aiut.h"
#include "aiut_log.h"
#include "sink/bluetooth_audio_render_sink.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t TEST_SAMPLE_RATE_48000 = 48000;
constexpr uint32_t TEST_CHANNEL_STEREO = 2;
constexpr float TEST_VOLUME_FULL = 1.0f;
constexpr size_t TEST_BUFFER_SIZE = 4096;
constexpr int32_t TEST_APP_UID_1 = 1001;
constexpr int32_t TEST_APP_UID_2 = 1002;
} // namespace

// ==================== BluetoothAudioRenderSinkAiut ====================

void BluetoothAudioRenderSinkAiut::SetUpTestCase(void) {}
void BluetoothAudioRenderSinkAiut::TearDownTestCase(void) {}
void BluetoothAudioRenderSinkAiut::SetUp(void) {}
void BluetoothAudioRenderSinkAiut::TearDown(void) {}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test BluetoothAudioRenderSink construction
 * @tc.number: BtSink_Constructor_001
 * @tc.desc  : Test BluetoothAudioRenderSink constructor creates uninitialized object.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "newly constructed sink should not be inited");
}

/**
 * @tc.name  : Test BluetoothAudioRenderSink destruction
 * @tc.number: BtSink_Destructor_001
 * @tc.desc  : Test BluetoothAudioRenderSink destructor on uninitialized sink does not crash.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink.reset();
}

/**
 * @tc.name  : Test BluetoothAudioRenderSink construction with low latency
 * @tc.number: BtSink_Constructor_LowLatency_001
 * @tc.desc  : Test BluetoothAudioRenderSink constructor with isBluetoothLowLatency=true.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_Constructor_LowLatency_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>(true, "a2dp");
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "newly constructed sink should not be inited");
}

// ==================== IsInited / DeInit Tests ====================

/**
 * @tc.name  : Test IsInited returns false before Init
 * @tc.number: BtSink_IsInited_001
 * @tc.desc  : Test IsInited returns false for newly constructed BluetoothAudioRenderSink.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_IsInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited before Init");
}

/**
 * @tc.name  : Test DeInit on uninitialized sink does not crash
 * @tc.number: BtSink_DeInit_NotInited_001
 * @tc.desc  : Test DeInit on uninitialized sink does not crash.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_DeInit_NotInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->DeInit();
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after DeInit");
}

// ==================== Lifecycle Tests ====================

/**
 * @tc.name  : Test Start/Stop/Pause/Resume/Flush/Reset on uninitialized sink
 * @tc.number: BtSink_Lifecycle_001
 * @tc.desc  : Test lifecycle methods on uninitialized bluetooth sink return errors.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_Lifecycle_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->Start();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "Start should fail when not inited, ret=%d", ret);
    ret = sink->Stop();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Stop should return SUCCESS, ret=%d", ret);
    ret = sink->Resume();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "Resume should fail when not inited, ret=%d", ret);
    ret = sink->Pause();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "Pause should fail when not inited, ret=%d", ret);
    ret = sink->Flush();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "Flush should fail when not inited, ret=%d", ret);
    ret = sink->Reset();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "Reset should fail when not inited, ret=%d", ret);
}

// ==================== RenderFrame Tests ====================

/**
 * @tc.name  : Test RenderFrame without Init returns error
 * @tc.number: BtSink_RenderFrame_NotInited_001
 * @tc.desc  : Test RenderFrame on uninitialized sink returns error.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_RenderFrame_NotInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    char data[TEST_BUFFER_SIZE] = {0};
    uint64_t writeLen = 0;
    int32_t ret = sink->RenderFrame(*data, TEST_BUFFER_SIZE, writeLen);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "RenderFrame without Init should fail, ret=%d", ret);
}

// ==================== Volume Tests ====================

/**
 * @tc.name  : Test SetVolume on uninitialized sink
 * @tc.number: BtSink_SetVolume_001
 * @tc.desc  : Test SetVolume on uninitialized bluetooth sink returns ERR_INVALID_HANDLE.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_SetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetVolume(TEST_VOLUME_FULL, TEST_VOLUME_FULL);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "SetVolume should return ERR_INVALID_HANDLE, ret=%d", ret);
}

/**
 * @tc.name  : Test GetVolume on uninitialized sink returns SUCCESS (default volume)
 * @tc.number: BtSink_GetVolume_001
 * @tc.desc  : Test GetVolume on uninitialized bluetooth sink returns SUCCESS.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_GetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    float left = 0.0f;
    float right = 0.0f;
    int32_t ret = sink->GetVolume(left, right);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetVolume should return SUCCESS, ret=%d", ret);
}

// ==================== Unsupported Interface Tests ====================

/**
 * @tc.name  : Test GetLatency on uninitialized sink
 * @tc.number: BtSink_GetLatency_001
 * @tc.desc  : Test GetLatency on uninitialized bluetooth sink returns error.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_GetLatency_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    uint32_t latency = 0;
    int32_t ret = sink->GetLatency(latency);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "GetLatency should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test GetMaxAmplitude returns 0.0
 * @tc.number: BtSink_GetMaxAmplitude_001
 * @tc.desc  : Test GetMaxAmplitude returns 0.0 on uninitialized bluetooth sink.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_GetMaxAmplitude_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    float amplitude = sink->GetMaxAmplitude();
    AIUT_EXPECT_EQ_INFO_LOG(amplitude, 0.0f, "GetMaxAmplitude should return 0.0, amplitude=%f", amplitude);
}

/**
 * @tc.name  : Test SetAudioScene returns ERR_NOT_SUPPORTED
 * @tc.number: BtSink_SetAudioScene_001
 * @tc.desc  : Test SetAudioScene returns ERR_NOT_SUPPORTED on bluetooth sink.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_SetAudioScene_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetAudioScene(AUDIO_SCENE_DEFAULT);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "SetAudioScene should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test UpdateActiveDevice returns ERR_NOT_SUPPORTED
 * @tc.number: BtSink_UpdateActiveDevice_001
 * @tc.desc  : Test UpdateActiveDevice returns ERR_NOT_SUPPORTED on bluetooth sink.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_UpdateActiveDevice_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::vector<DeviceType> devices = { DEVICE_TYPE_SPEAKER };
    int32_t ret = sink->UpdateActiveDevice(devices);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "UpdateActiveDevice should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

// ==================== AudioMono / AudioBalance Tests ====================

/**
 * @tc.name  : Test SetAudioMonoState does not crash
 * @tc.number: BtSink_SetAudioMonoState_001
 * @tc.desc  : Test SetAudioMonoState with true and false does not crash.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_SetAudioMonoState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetAudioMonoState(true);
    sink->SetAudioMonoState(false);
}

/**
 * @tc.name  : Test SetAudioBalanceValue does not crash
 * @tc.number: BtSink_SetAudioBalanceValue_001
 * @tc.desc  : Test SetAudioBalanceValue with various values does not crash.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_SetAudioBalanceValue_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetAudioBalanceValue(0.0f);
    sink->SetAudioBalanceValue(-1.0f);
    sink->SetAudioBalanceValue(1.0f);
}

// ==================== SuspendRenderSink / RestoreRenderSink Tests ====================

/**
 * @tc.name  : Test SuspendRenderSink on uninitialized sink
 * @tc.number: BtSink_SuspendRenderSink_001
 * @tc.desc  : Test SuspendRenderSink on uninitialized bluetooth sink returns error.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_SuspendRenderSink_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SuspendRenderSink();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "SuspendRenderSink should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test RestoreRenderSink on uninitialized sink
 * @tc.number: BtSink_RestoreRenderSink_001
 * @tc.desc  : Test RestoreRenderSink on uninitialized bluetooth sink returns error.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_RestoreRenderSink_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->RestoreRenderSink();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "RestoreRenderSink should fail, ret=%d", ret);
}

// ==================== SetSinkMuteForSwitchDevice Tests ====================

/**
 * @tc.name  : Test SetSinkMuteForSwitchDevice on uninitialized sink
 * @tc.number: BtSink_SetSinkMuteForSwitchDevice_001
 * @tc.desc  : Test SetSinkMuteForSwitchDevice on uninitialized sink returns SUCCESS.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_SetSinkMuteForSwitchDevice_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetSinkMuteForSwitchDevice(true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SetSinkMuteForSwitchDevice should return SUCCESS, ret=%d", ret);
}

// ==================== UpdateAppsUid Tests ====================

/**
 * @tc.name  : Test UpdateAppsUid with array input
 * @tc.number: BtSink_UpdateAppsUid_Array_001
 * @tc.desc  : Test UpdateAppsUid with array input returns SUCCESS.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_UpdateAppsUid_Array_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t appsUid[MAX_MIX_CHANNELS] = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid, MAX_MIX_CHANNELS);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "UpdateAppsUid array should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test UpdateAppsUid with vector input
 * @tc.number: BtSink_UpdateAppsUid_Vector_001
 * @tc.desc  : Test UpdateAppsUid with vector input returns SUCCESS.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_UpdateAppsUid_Vector_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::vector<int32_t> appsUid = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "UpdateAppsUid vector should return SUCCESS, ret=%d", ret);
}

// ==================== SetInvalidState Tests ====================

/**
 * @tc.name  : Test SetInvalidState does not crash
 * @tc.number: BtSink_SetInvalidState_001
 * @tc.desc  : Test SetInvalidState on bluetooth sink does not crash.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_SetInvalidState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetInvalidState();
}

// ==================== DumpInfo Tests ====================

/**
 * @tc.name  : Test DumpInfo on uninitialized sink
 * @tc.number: BtSink_DumpInfo_001
 * @tc.desc  : Test DumpInfo on uninitialized sink does not crash.
 */
HWTEST_F(BluetoothAudioRenderSinkAiut, BtSink_DumpInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<BluetoothAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::string dumpString;
    sink->DumpInfo(dumpString);
}

} // namespace AudioStandard
} // namespace OHOS
