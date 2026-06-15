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

#include "direct_audio_render_sink_aiut.h"
#include "aiut_log.h"
#include "sink/direct_audio_render_sink.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr float TEST_VOLUME_FULL = 1.0f;
constexpr size_t TEST_BUFFER_SIZE = 4096;
constexpr int32_t TEST_APP_UID_1 = 1001;
constexpr int32_t TEST_APP_UID_2 = 1002;
} // namespace

void DirectAudioRenderSinkAiut::SetUpTestCase(void) {}
void DirectAudioRenderSinkAiut::TearDownTestCase(void) {}
void DirectAudioRenderSinkAiut::SetUp(void) {}
void DirectAudioRenderSinkAiut::TearDown(void) {}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test DirectAudioRenderSink construction
 * @tc.number: DirectSink_Constructor_001
 * @tc.desc  : Test DirectAudioRenderSink constructor creates uninitialized object.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "newly constructed sink should not be inited");
}

/**
 * @tc.name  : Test DirectAudioRenderSink destruction
 * @tc.number: DirectSink_Destructor_001
 * @tc.desc  : Test DirectAudioRenderSink destructor on uninitialized sink does not crash.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink.reset();
}

// ==================== IsInited / DeInit Tests ====================

/**
 * @tc.name  : Test IsInited returns false before Init
 * @tc.number: DirectSink_IsInited_001
 * @tc.desc  : Test IsInited returns false for newly constructed DirectAudioRenderSink.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_IsInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited before Init");
}

/**
 * @tc.name  : Test DeInit on uninitialized sink does not crash
 * @tc.number: DirectSink_DeInit_NotInited_001
 * @tc.desc  : Test DeInit on uninitialized sink does not crash.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_DeInit_NotInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->DeInit();
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after DeInit");
}

// ==================== Lifecycle Tests ====================

/**
 * @tc.name  : Test Start/Stop/Pause/Resume/Flush/Reset on uninitialized sink
 * @tc.number: DirectSink_Lifecycle_001
 * @tc.desc  : Test lifecycle methods on uninitialized direct sink return errors.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_Lifecycle_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->Start();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "Start should fail when not inited, ret=%d", ret);
    ret = sink->Stop();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Stop should return SUCCESS, ret=%d", ret);
    ret = sink->Resume();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "Resume should return ERR_NOT_SUPPORTED when not inited, ret=%d", ret);
    ret = sink->Pause();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "Pause should fail when not inited, ret=%d", ret);
    ret = sink->Flush();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "Flush should fail when not inited, ret=%d", ret);
    ret = sink->Reset();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Reset should return SUCCESS, ret=%d", ret);
}

// ==================== RenderFrame Tests ====================

/**
 * @tc.name  : Test RenderFrame without Init returns error
 * @tc.number: DirectSink_RenderFrame_NotInited_001
 * @tc.desc  : Test RenderFrame on uninitialized sink returns error.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_RenderFrame_NotInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
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
 * @tc.number: DirectSink_SetVolume_001
 * @tc.desc  : Test SetVolume on uninitialized direct sink returns ERR_INVALID_HANDLE.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_SetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetVolume(TEST_VOLUME_FULL, TEST_VOLUME_FULL);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "SetVolume should return ERR_INVALID_HANDLE, ret=%d", ret);
}

/**
 * @tc.name  : Test GetVolume on uninitialized sink returns SUCCESS
 * @tc.number: DirectSink_GetVolume_001
 * @tc.desc  : Test GetVolume on uninitialized direct sink returns SUCCESS (default volume).
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_GetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    float left = 0.0f;
    float right = 0.0f;
    int32_t ret = sink->GetVolume(left, right);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetVolume should return SUCCESS, ret=%d", ret);
}

// ==================== SetSinkMuteForSwitchDevice Tests ====================

/**
 * @tc.name  : Test SetSinkMuteForSwitchDevice on uninitialized sink
 * @tc.number: DirectSink_SetSinkMuteForSwitchDevice_001
 * @tc.desc  : Test SetSinkMuteForSwitchDevice on uninitialized sink returns SUCCESS.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_SetSinkMuteForSwitchDevice_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetSinkMuteForSwitchDevice(true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SetSinkMuteForSwitchDevice should return SUCCESS, ret=%d", ret);
}

// ==================== UpdateAppsUid Tests ====================

/**
 * @tc.name  : Test UpdateAppsUid with array input
 * @tc.number: DirectSink_UpdateAppsUid_Array_001
 * @tc.desc  : Test UpdateAppsUid with array input returns SUCCESS.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_UpdateAppsUid_Array_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t appsUid[MAX_MIX_CHANNELS] = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid, MAX_MIX_CHANNELS);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "UpdateAppsUid array should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test UpdateAppsUid with vector input
 * @tc.number: DirectSink_UpdateAppsUid_Vector_001
 * @tc.desc  : Test UpdateAppsUid with vector input returns SUCCESS.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_UpdateAppsUid_Vector_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::vector<int32_t> appsUid = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "UpdateAppsUid vector should return SUCCESS, ret=%d", ret);
}

// ==================== AudioMono / AudioBalance Tests ====================

/**
 * @tc.name  : Test SetAudioMonoState does not crash
 * @tc.number: DirectSink_SetAudioMonoState_001
 * @tc.desc  : Test SetAudioMonoState with true and false does not crash.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_SetAudioMonoState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetAudioMonoState(true);
    sink->SetAudioMonoState(false);
}

/**
 * @tc.name  : Test SetAudioBalanceValue does not crash
 * @tc.number: DirectSink_SetAudioBalanceValue_001
 * @tc.desc  : Test SetAudioBalanceValue with various values does not crash.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_SetAudioBalanceValue_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetAudioBalanceValue(0.0f);
    sink->SetAudioBalanceValue(-1.0f);
    sink->SetAudioBalanceValue(1.0f);
}

// ==================== DumpInfo / GetMaxAmplitude Tests ====================

/**
 * @tc.name  : Test DumpInfo on uninitialized sink
 * @tc.number: DirectSink_DumpInfo_001
 * @tc.desc  : Test DumpInfo on uninitialized sink does not crash.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_DumpInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::string dumpString;
    sink->DumpInfo(dumpString);
}

/**
 * @tc.name  : Test GetMaxAmplitude returns 0.0
 * @tc.number: DirectSink_GetMaxAmplitude_001
 * @tc.desc  : Test GetMaxAmplitude returns 0.0 on uninitialized sink.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_GetMaxAmplitude_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    float amplitude = sink->GetMaxAmplitude();
    AIUT_EXPECT_EQ_INFO_LOG(amplitude, 0.0f, "GetMaxAmplitude should return 0.0, amplitude=%f", amplitude);
}

/**
 * @tc.name  : Test GetVolumeDataCount returns 0
 * @tc.number: DirectSink_GetVolumeDataCount_001
 * @tc.desc  : Test GetVolumeDataCount returns 0 on uninitialized sink.
 */
HWTEST_F(DirectAudioRenderSinkAiut, DirectSink_GetVolumeDataCount_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<DirectAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int64_t volumeData = -1;
    int32_t ret = sink->GetVolumeDataCount(volumeData);
    AIUT_EXPECT_EQ_INFO_LOG(ret, 0, "GetVolumeDataCount should return 0, ret=%d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
