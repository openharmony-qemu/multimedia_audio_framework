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

#include "audio_render_sink_aiut.h"
#include "aiut_log.h"
#include "sink/audio_render_sink.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t TEST_RENDER_ID = 0;
constexpr uint32_t TEST_SAMPLE_RATE_48000 = 48000;
constexpr uint32_t TEST_CHANNEL_STEREO = 2;
constexpr uint32_t TEST_CHANNEL_LAYOUT_STEREO = 3;
constexpr float TEST_VOLUME_FULL = 1.0f;
constexpr float TEST_VOLUME_HALF = 0.5f;
constexpr size_t TEST_BUFFER_SIZE = 4096;
constexpr int32_t TEST_APP_UID_1 = 1001;
constexpr int32_t TEST_APP_UID_2 = 1002;
} // namespace

// ==================== AudioRenderSinkAiut ====================

void AudioRenderSinkAiut::SetUpTestCase(void) {}
void AudioRenderSinkAiut::TearDownTestCase(void) {}
void AudioRenderSinkAiut::SetUp(void) {}
void AudioRenderSinkAiut::TearDown(void) {}

IAudioSinkAttr AudioRenderSinkAiut::CreatePrimaryAttr()
{
    IAudioSinkAttr attr = {};
    attr.adapterName = "primary";
    attr.sinkName = "speaker";
    attr.sampleRate = TEST_SAMPLE_RATE_48000;
    attr.channel = TEST_CHANNEL_STEREO;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = TEST_CHANNEL_LAYOUT_STEREO;
    attr.deviceType = DEVICE_TYPE_SPEAKER;
    attr.volume = TEST_VOLUME_FULL;
    attr.openMicSpeaker = 1;
    return attr;
}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test AudioRenderSink construction with renderId
 * @tc.number: AudioRenderSink_Constructor_001
 * @tc.desc  : Test AudioRenderSink constructor creates uninitialized object.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "newly constructed sink should not be inited");
}

/**
 * @tc.name  : Test AudioRenderSink destruction with uninitialized sink
 * @tc.number: AudioRenderSink_Destructor_001
 * @tc.desc  : Test AudioRenderSink destructor on uninitialized sink does not crash.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink.reset();
}

// ==================== Init / DeInit Tests ====================

/**
 * @tc.name  : Test Init on uninitialized sink
 * @tc.number: AudioRenderSink_Init_001
 * @tc.desc  : Test Init on newly constructed sink (depends on HDI availability).
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_Init_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreatePrimaryAttr();
    int32_t ret = sink->Init(attr);
    // Result depends on HDI availability; just verify no crash
    AIUT_INFO_LOG("Init returned %d", ret);
    if (sink->IsInited()) {
        sink->DeInit();
    }
}

/**
 * @tc.name  : Test DeInit on uninitialized sink does not crash
 * @tc.number: AudioRenderSink_DeInit_NotInited_001
 * @tc.desc  : Test DeInit on uninitialized sink does not crash.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_DeInit_NotInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->DeInit();
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after DeInit");
}

// ==================== IsInited Tests ====================

/**
 * @tc.name  : Test IsInited returns false before Init
 * @tc.number: AudioRenderSink_IsInited_001
 * @tc.desc  : Test IsInited returns false for newly constructed AudioRenderSink.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_IsInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited before Init");
}

// ==================== Start / Stop / Pause / Resume / Flush / Reset Tests ====================

/**
 * @tc.name  : Test Start returns error when not inited
 * @tc.number: AudioRenderSink_Start_001
 * @tc.desc  : Test Start on uninitialized sink returns error.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_Start_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->Start();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "Start on uninitied sink should not return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test Stop on uninitialized sink
 * @tc.number: AudioRenderSink_Stop_001
 * @tc.desc  : Test Stop on uninitialized sink returns SUCCESS or error.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_Stop_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->Stop();
}

/**
 * @tc.name  : Test Pause/Resume/Flush/Reset on uninitialized sink
 * @tc.number: AudioRenderSink_Lifecycle_001
 * @tc.desc  : Test lifecycle methods on uninitialized sink do not crash.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_Lifecycle_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->Pause();
    sink->Resume();
    sink->Flush();
    sink->Reset();
}

// ==================== RenderFrame Tests ====================

/**
 * @tc.name  : Test RenderFrame without Init returns error
 * @tc.number: AudioRenderSink_RenderFrame_NotInited_001
 * @tc.desc  : Test RenderFrame on uninitialized sink returns ERROR_ILLEGAL_STATE.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_RenderFrame_NotInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    char data[TEST_BUFFER_SIZE] = {0};
    uint64_t writeLen = 0;
    int32_t ret = sink->RenderFrame(*data, TEST_BUFFER_SIZE, writeLen);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR_ILLEGAL_STATE,
        "RenderFrame without Init should return ERROR_ILLEGAL_STATE, ret=%d", ret);
}

// ==================== Volume Tests ====================

/**
 * @tc.name  : Test SetVolume returns error when not inited
 * @tc.number: AudioRenderSink_SetVolume_001
 * @tc.desc  : Test SetVolume on uninitialized sink returns error.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_SetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetVolume(TEST_VOLUME_FULL, TEST_VOLUME_FULL);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "SetVolume on uninitied sink should not return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test SetVolumeWithRamp returns error when not inited
 * @tc.number: AudioRenderSink_SetVolumeWithRamp_001
 * @tc.desc  : Test SetVolumeWithRamp on uninitialized sink returns error.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_SetVolumeWithRamp_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetVolumeWithRamp(TEST_VOLUME_HALF, TEST_VOLUME_HALF, 100);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "SetVolumeWithRamp on uninitied sink should not return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test GetVolume on uninitialized sink
 * @tc.number: AudioRenderSink_GetVolume_001
 * @tc.desc  : Test GetVolume on uninitialized sink returns error.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_GetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    float left = 0.0f;
    float right = 0.0f;
    int32_t ret = sink->GetVolume(left, right);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "GetVolume on uninitied sink should not return SUCCESS, ret=%d", ret);
}

// ==================== Latency / Position Tests ====================

/**
 * @tc.name  : Test GetLatency on uninitialized sink
 * @tc.number: AudioRenderSink_GetLatency_001
 * @tc.desc  : Test GetLatency on uninitialized sink returns error.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_GetLatency_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    uint32_t latency = 0;
    int32_t ret = sink->GetLatency(latency);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "GetLatency on uninitied sink should not return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test GetPresentationPosition on uninitialized sink
 * @tc.number: AudioRenderSink_GetPresentationPosition_001
 * @tc.desc  : Test GetPresentationPosition on uninitialized sink returns error.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_GetPresentationPosition_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int32_t ret = sink->GetPresentationPosition(frames, timeSec, timeNanoSec);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "GetPresentationPosition on uninitied sink should not return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test GetMaxAmplitude returns 0.0
 * @tc.number: AudioRenderSink_GetMaxAmplitude_001
 * @tc.desc  : Test GetMaxAmplitude returns 0.0 on uninitialized sink.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_GetMaxAmplitude_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    float amplitude = sink->GetMaxAmplitude();
    AIUT_EXPECT_EQ_INFO_LOG(amplitude, 0.0f,
        "GetMaxAmplitude should return 0.0, amplitude=%f", amplitude);
}

// ==================== AudioMono / AudioBalance Tests ====================

/**
 * @tc.name  : Test SetAudioMonoState does not crash
 * @tc.number: AudioRenderSink_SetAudioMonoState_001
 * @tc.desc  : Test SetAudioMonoState with true and false does not crash.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_SetAudioMonoState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetAudioMonoState(true);
    sink->SetAudioMonoState(false);
}

/**
 * @tc.name  : Test SetAudioBalanceValue does not crash
 * @tc.number: AudioRenderSink_SetAudioBalanceValue_001
 * @tc.desc  : Test SetAudioBalanceValue with various values does not crash.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_SetAudioBalanceValue_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetAudioBalanceValue(0.0f);
    sink->SetAudioBalanceValue(-1.0f);
    sink->SetAudioBalanceValue(1.0f);
}

// ==================== SetSinkMuteForSwitchDevice Tests ====================

/**
 * @tc.name  : Test SetSinkMuteForSwitchDevice on uninitialized sink
 * @tc.number: AudioRenderSink_SetSinkMuteForSwitchDevice_001
 * @tc.desc  : Test SetSinkMuteForSwitchDevice on uninitialized sink returns SUCCESS.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_SetSinkMuteForSwitchDevice_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetSinkMuteForSwitchDevice(true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SetSinkMuteForSwitchDevice should return SUCCESS, ret=%d", ret);
    ret = sink->SetSinkMuteForSwitchDevice(false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SetSinkMuteForSwitchDevice(false) should return SUCCESS, ret=%d", ret);
}

// ==================== UpdateAppsUid Tests ====================

/**
 * @tc.name  : Test UpdateAppsUid with array input
 * @tc.number: AudioRenderSink_UpdateAppsUid_Array_001
 * @tc.desc  : Test UpdateAppsUid with array input returns SUCCESS.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_UpdateAppsUid_Array_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t appsUid[MAX_MIX_CHANNELS] = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid, MAX_MIX_CHANNELS);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "UpdateAppsUid array should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test UpdateAppsUid with vector input
 * @tc.number: AudioRenderSink_UpdateAppsUid_Vector_001
 * @tc.desc  : Test UpdateAppsUid with vector input returns SUCCESS.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_UpdateAppsUid_Vector_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::vector<int32_t> appsUid = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "UpdateAppsUid vector should return SUCCESS, ret=%d", ret);
}

// ==================== GetVolumeDataCount Tests ====================

/**
 * @tc.name  : Test GetVolumeDataCount returns 0
 * @tc.number: AudioRenderSink_GetVolumeDataCount_001
 * @tc.desc  : Test GetVolumeDataCount returns 0 on uninitialized sink.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_GetVolumeDataCount_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int64_t volumeData = -1;
    int32_t ret = sink->GetVolumeDataCount(volumeData);
    AIUT_EXPECT_EQ_INFO_LOG(ret, 0, "GetVolumeDataCount should return 0, ret=%d", ret);
}

// ==================== DumpInfo Tests ====================

/**
 * @tc.name  : Test DumpInfo on uninitialized sink
 * @tc.number: AudioRenderSink_DumpInfo_001
 * @tc.desc  : Test DumpInfo on uninitialized sink does not crash.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_DumpInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::string dumpString;
    sink->DumpInfo(dumpString);
}

// ==================== GetCurrentOutputDevice Tests ====================

/**
 * @tc.name  : Test GetCurrentOutputDevice on uninitialized sink
 * @tc.number: AudioRenderSink_GetCurrentOutputDevice_001
 * @tc.desc  : Test GetCurrentOutputDevice on uninitialized sink returns DEVICE_TYPE_NONE.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_GetCurrentOutputDevice_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    DeviceType deviceType = sink->GetCurrentOutputDevice();
    AIUT_EXPECT_EQ_INFO_LOG(deviceType, DEVICE_TYPE_NONE,
        "GetCurrentOutputDevice should return DEVICE_TYPE_NONE, deviceType=%d", deviceType);
}

// ==================== SetAudioScene Tests ====================

/**
 * @tc.name  : Test SetAudioScene on uninitialized sink
 * @tc.number: AudioRenderSink_SetAudioScene_001
 * @tc.desc  : Test SetAudioScene on uninitialized sink returns error.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_SetAudioScene_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetAudioScene(AUDIO_SCENE_DEFAULT);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "SetAudioScene on uninitied sink should not return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test IsInA2dpOffload returns false
 * @tc.number: AudioRenderSink_IsInA2dpOffload_001
 * @tc.desc  : Test IsInA2dpOffload returns false on AudioRenderSink.
 */
HWTEST_F(AudioRenderSinkAiut, AudioRenderSink_IsInA2dpOffload_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AudioRenderSink>(TEST_RENDER_ID);
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInA2dpOffload(), "IsInA2dpOffload should return false");
}

} // namespace AudioStandard
} // namespace OHOS
