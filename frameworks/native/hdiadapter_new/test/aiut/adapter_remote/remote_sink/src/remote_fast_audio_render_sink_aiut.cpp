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

#include "remote_fast_audio_render_sink_aiut.h"
#include "aiut_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr const char *DEVICE_NETWORK_ID = "test_network_id";
constexpr uint32_t TEST_CHANNEL = 2;
constexpr uint32_t TEST_SAMPLE_RATE = 48000;
}

void RemoteFastAudioRenderSinkAiut::SetUpTestCase(void) {}
void RemoteFastAudioRenderSinkAiut::TearDownTestCase(void) {}

void RemoteFastAudioRenderSinkAiut::SetUp(void)
{
    sink_ = std::make_shared<RemoteFastAudioRenderSink>(DEVICE_NETWORK_ID);
    attr_.adapterName = "remote_fast_test";
    attr_.channel = TEST_CHANNEL;
    attr_.sampleRate = TEST_SAMPLE_RATE;
    attr_.format = SAMPLE_S16LE;
}

void RemoteFastAudioRenderSinkAiut::TearDown(void)
{
    if (sink_ && sink_->IsInited()) {
        sink_->DeInit();
    }
    sink_ = nullptr;
}

/**
 * @tc.name  : RemoteFastAudioRenderSink_IsInited_001
 * @tc.number: RemoteFastAudioRenderSink_IsInited_001
 * @tc.desc  : Test IsInited on fresh sink, should be false
 */
AIUT_F(RemoteFastAudioRenderSinkAiut, RemoteFastAudioRenderSink_IsInited_001)
{
    AIUT_INFO_LOG("RemoteFastAudioRenderSink_IsInited_001 start");
    ASSERT_NE(sink_, nullptr);
    EXPECT_FALSE(sink_->IsInited());
}

/**
 * @tc.name  : RemoteFastAudioRenderSink_DeInit_001
 * @tc.number: RemoteFastAudioRenderSink_DeInit_001
 * @tc.desc  : Test DeInit on uninitialized sink
 */
AIUT_F(RemoteFastAudioRenderSinkAiut, RemoteFastAudioRenderSink_DeInit_001)
{
    AIUT_INFO_LOG("RemoteFastAudioRenderSink_DeInit_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->DeInit();
    EXPECT_FALSE(sink_->IsInited());
}

/**
 * @tc.name  : RemoteFastAudioRenderSink_Start_001
 * @tc.number: RemoteFastAudioRenderSink_Start_001
 * @tc.desc  : Test Start on uninitialized sink
 */
AIUT_F(RemoteFastAudioRenderSinkAiut, RemoteFastAudioRenderSink_Start_001)
{
    AIUT_INFO_LOG("RemoteFastAudioRenderSink_Start_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Start();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteFastAudioRenderSink_Stop_001
 * @tc.number: RemoteFastAudioRenderSink_Stop_001
 * @tc.desc  : Test Stop on uninitialized sink
 */
AIUT_F(RemoteFastAudioRenderSinkAiut, RemoteFastAudioRenderSink_Stop_001)
{
    AIUT_INFO_LOG("RemoteFastAudioRenderSink_Stop_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteFastAudioRenderSink_Pause_001
 * @tc.number: RemoteFastAudioRenderSink_Pause_001
 * @tc.desc  : Test Pause on uninitialized sink
 */
AIUT_F(RemoteFastAudioRenderSinkAiut, RemoteFastAudioRenderSink_Pause_001)
{
    AIUT_INFO_LOG("RemoteFastAudioRenderSink_Pause_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Pause();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteFastAudioRenderSink_Resume_001
 * @tc.number: RemoteFastAudioRenderSink_Resume_001
 * @tc.desc  : Test Resume on uninitialized sink
 */
AIUT_F(RemoteFastAudioRenderSinkAiut, RemoteFastAudioRenderSink_Resume_001)
{
    AIUT_INFO_LOG("RemoteFastAudioRenderSink_Resume_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Resume();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteFastAudioRenderSink_Flush_001
 * @tc.number: RemoteFastAudioRenderSink_Flush_001
 * @tc.desc  : Test Flush on uninitialized sink
 */
AIUT_F(RemoteFastAudioRenderSinkAiut, RemoteFastAudioRenderSink_Flush_001)
{
    AIUT_INFO_LOG("RemoteFastAudioRenderSink_Flush_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Flush();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteFastAudioRenderSink_SetVolume_001
 * @tc.number: RemoteFastAudioRenderSink_SetVolume_001
 * @tc.desc  : Test SetVolume on uninitialized sink
 */
AIUT_F(RemoteFastAudioRenderSinkAiut, RemoteFastAudioRenderSink_SetVolume_001)
{
    AIUT_INFO_LOG("RemoteFastAudioRenderSink_SetVolume_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->SetVolume(1.0f, 1.0f);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteFastAudioRenderSink_GetVolume_001
 * @tc.number: RemoteFastAudioRenderSink_GetVolume_001
 * @tc.desc  : Test GetVolume on uninitialized sink
 */
AIUT_F(RemoteFastAudioRenderSinkAiut, RemoteFastAudioRenderSink_GetVolume_001)
{
    AIUT_INFO_LOG("RemoteFastAudioRenderSink_GetVolume_001 start");
    ASSERT_NE(sink_, nullptr);
    float left = 0.0f;
    float right = 0.0f;
    int32_t ret = sink_->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteFastAudioRenderSink_SetInvalidState_001
 * @tc.number: RemoteFastAudioRenderSink_SetInvalidState_001
 * @tc.desc  : Test SetInvalidState on sink
 */
AIUT_F(RemoteFastAudioRenderSinkAiut, RemoteFastAudioRenderSink_SetInvalidState_001)
{
    AIUT_INFO_LOG("RemoteFastAudioRenderSink_SetInvalidState_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->SetInvalidState();
    EXPECT_FALSE(sink_->validState_.load());
}

/**
 * @tc.name  : RemoteFastAudioRenderSink_UpdateAppsUid_001
 * @tc.number: RemoteFastAudioRenderSink_UpdateAppsUid_001
 * @tc.desc  : Test UpdateAppsUid with vector
 */
AIUT_F(RemoteFastAudioRenderSinkAiut, RemoteFastAudioRenderSink_UpdateAppsUid_001)
{
    AIUT_INFO_LOG("RemoteFastAudioRenderSink_UpdateAppsUid_001 start");
    ASSERT_NE(sink_, nullptr);
    std::vector<int32_t> appsUid = {20000001, 20000002};
    int32_t ret = sink_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);
    appsUid.clear();
    ret = sink_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
