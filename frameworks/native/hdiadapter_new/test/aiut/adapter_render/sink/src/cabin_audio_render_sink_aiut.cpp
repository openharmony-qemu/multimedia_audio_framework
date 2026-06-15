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

#include "cabin_audio_render_sink_aiut.h"
#include "aiut_log.h"
#include "sink/cabin_audio_render_sink.h"
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

void CabinAudioRenderSinkAiut::SetUpTestCase(void) {}
void CabinAudioRenderSinkAiut::TearDownTestCase(void) {}
void CabinAudioRenderSinkAiut::SetUp(void) {}
void CabinAudioRenderSinkAiut::TearDown(void) {}

// ==================== Construction / Destruction ====================

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "newly constructed sink should not be inited");
}

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink.reset();
}

// ==================== IsInited / DeInit ====================

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_IsInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited before Init");
}

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_DeInit_NotInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->DeInit();
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after DeInit");
}

// ==================== Lifecycle ====================

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_Lifecycle_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->Start();
    sink->Stop();
    sink->Pause();
    sink->Resume();
    sink->Flush();
    sink->Reset();
}

// ==================== RenderFrame ====================

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_RenderFrame_NotInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    char data[TEST_BUFFER_SIZE] = {0};
    uint64_t writeLen = 0;
    int32_t ret = sink->RenderFrame(*data, TEST_BUFFER_SIZE, writeLen);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "RenderFrame without Init should fail, ret=%d", ret);
}

// ==================== Volume ====================

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_SetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetVolume(TEST_VOLUME_FULL, TEST_VOLUME_FULL);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "SetVolume should return ERR_INVALID_HANDLE, ret=%d", ret);
}

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_GetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    float left = 0.0f;
    float right = 0.0f;
    int32_t ret = sink->GetVolume(left, right);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetVolume should return SUCCESS, ret=%d", ret);
}

// ==================== SetSinkMuteForSwitchDevice ====================

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_SetSinkMuteForSwitchDevice_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetSinkMuteForSwitchDevice(true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SetSinkMuteForSwitchDevice should return SUCCESS, ret=%d", ret);
    ret = sink->SetSinkMuteForSwitchDevice(false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SetSinkMuteForSwitchDevice(false) should return SUCCESS, ret=%d", ret);
}

// ==================== UpdateAppsUid ====================

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_UpdateAppsUid_Array_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t appsUid[MAX_MIX_CHANNELS] = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid, MAX_MIX_CHANNELS);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "UpdateAppsUid array should return SUCCESS, ret=%d", ret);
}

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_UpdateAppsUid_Vector_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::vector<int32_t> appsUid = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "UpdateAppsUid vector should return SUCCESS, ret=%d", ret);
}

// ==================== Misc ====================

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_SetAudioMonoState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetAudioMonoState(true);
    sink->SetAudioMonoState(false);
}

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_SetAudioBalanceValue_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetAudioBalanceValue(0.0f);
    sink->SetAudioBalanceValue(-1.0f);
    sink->SetAudioBalanceValue(1.0f);
}

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_DumpInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::string dumpString;
    sink->DumpInfo(dumpString);
}

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_GetMaxAmplitude_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    float amplitude = sink->GetMaxAmplitude();
    AIUT_EXPECT_EQ_INFO_LOG(amplitude, 0.0f, "GetMaxAmplitude should return 0.0, amplitude=%f", amplitude);
}

HWTEST_F(CabinAudioRenderSinkAiut, CabinSink_GetVolumeDataCount_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<CabinAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int64_t volumeData = -1;
    int32_t ret = sink->GetVolumeDataCount(volumeData);
    AIUT_EXPECT_EQ_INFO_LOG(ret, 0, "GetVolumeDataCount should return 0, ret=%d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
