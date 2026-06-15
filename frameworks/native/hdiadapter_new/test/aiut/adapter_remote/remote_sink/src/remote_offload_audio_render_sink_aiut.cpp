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

#include "remote_offload_audio_render_sink_aiut.h"
#include "aiut_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr const char *DEVICE_NETWORK_ID = "test_network_id";
constexpr uint32_t TEST_CHANNEL = 2;
constexpr uint32_t TEST_SAMPLE_RATE = 48000;
}

void RemoteOffloadAudioRenderSinkAiut::SetUpTestCase(void) {}
void RemoteOffloadAudioRenderSinkAiut::TearDownTestCase(void) {}

void RemoteOffloadAudioRenderSinkAiut::SetUp(void)
{
    sink_ = std::make_shared<RemoteOffloadAudioRenderSink>(DEVICE_NETWORK_ID);
    attr_.adapterName = "remote_offload_test";
    attr_.channel = TEST_CHANNEL;
    attr_.sampleRate = TEST_SAMPLE_RATE;
    attr_.format = SAMPLE_S16LE;
}

void RemoteOffloadAudioRenderSinkAiut::TearDown(void)
{
    if (sink_ && sink_->IsInited()) {
        sink_->DeInit();
    }
    sink_ = nullptr;
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_IsInited_001
 * @tc.number: RemoteOffloadAudioRenderSink_IsInited_001
 * @tc.desc  : Test IsInited on fresh sink, should be false
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_IsInited_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_IsInited_001 start");
    ASSERT_NE(sink_, nullptr);
    EXPECT_FALSE(sink_->IsInited());
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_DeInit_001
 * @tc.number: RemoteOffloadAudioRenderSink_DeInit_001
 * @tc.desc  : Test DeInit on uninitialized sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_DeInit_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_DeInit_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->DeInit();
    EXPECT_FALSE(sink_->IsInited());
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_Start_001
 * @tc.number: RemoteOffloadAudioRenderSink_Start_001
 * @tc.desc  : Test Start on uninitialized sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_Start_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_Start_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Start();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_Stop_001
 * @tc.number: RemoteOffloadAudioRenderSink_Stop_001
 * @tc.desc  : Test Stop on uninitialized sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_Stop_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_Stop_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_Pause_001
 * @tc.number: RemoteOffloadAudioRenderSink_Pause_001
 * @tc.desc  : Test Pause on uninitialized sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_Pause_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_Pause_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Pause();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_Resume_001
 * @tc.number: RemoteOffloadAudioRenderSink_Resume_001
 * @tc.desc  : Test Resume on uninitialized sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_Resume_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_Resume_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Resume();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_Flush_001
 * @tc.number: RemoteOffloadAudioRenderSink_Flush_001
 * @tc.desc  : Test Flush on uninitialized sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_Flush_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_Flush_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Flush();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_SetVolume_001
 * @tc.number: RemoteOffloadAudioRenderSink_SetVolume_001
 * @tc.desc  : Test SetVolume on uninitialized sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_SetVolume_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_SetVolume_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->SetVolume(1.0f, 1.0f);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_GetVolume_001
 * @tc.number: RemoteOffloadAudioRenderSink_GetVolume_001
 * @tc.desc  : Test GetVolume on uninitialized sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_GetVolume_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_GetVolume_001 start");
    ASSERT_NE(sink_, nullptr);
    float left = 0.0f;
    float right = 0.0f;
    int32_t ret = sink_->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_SetInvalidState_001
 * @tc.number: RemoteOffloadAudioRenderSink_SetInvalidState_001
 * @tc.desc  : Test SetInvalidState on sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_SetInvalidState_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_SetInvalidState_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->SetInvalidState();
    EXPECT_FALSE(sink_->validState_.load());
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_UpdateAppsUid_001
 * @tc.number: RemoteOffloadAudioRenderSink_UpdateAppsUid_001
 * @tc.desc  : Test UpdateAppsUid with vector
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_UpdateAppsUid_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_UpdateAppsUid_001 start");
    ASSERT_NE(sink_, nullptr);
    std::vector<int32_t> appsUid = {20000001, 20000002};
    int32_t ret = sink_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);
    appsUid.clear();
    ret = sink_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_Drain_001
 * @tc.number: RemoteOffloadAudioRenderSink_Drain_001
 * @tc.desc  : Test Drain on uninitialized sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_Drain_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_Drain_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Drain(AUDIO_DRAIN_NORMAL);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_SetSpeed_001
 * @tc.number: RemoteOffloadAudioRenderSink_SetSpeed_001
 * @tc.desc  : Test SetSpeed on uninitialized sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_SetSpeed_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_SetSpeed_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->SetSpeed(1.0f);
    // SetSpeed does not return error, just verify no crash
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_LockOffloadRunningLock_001
 * @tc.number: RemoteOffloadAudioRenderSink_LockOffloadRunningLock_001
 * @tc.desc  : Test LockOffloadRunningLock on uninitialized sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_LockOffloadRunningLock_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_LockOffloadRunningLock_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->LockOffloadRunningLock();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_UnLockOffloadRunningLock_001
 * @tc.number: RemoteOffloadAudioRenderSink_UnLockOffloadRunningLock_001
 * @tc.desc  : Test UnLockOffloadRunningLock on uninitialized sink
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_UnLockOffloadRunningLock_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_UnLockOffloadRunningLock_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->UnLockOffloadRunningLock();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_SetAudioScene_001
 * @tc.number: RemoteOffloadAudioRenderSink_SetAudioScene_001
 * @tc.desc  : Test SetAudioScene on uninitialized sink, should return NOT_SUPPORTED
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_SetAudioScene_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_SetAudioScene_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : RemoteOffloadAudioRenderSink_UpdateActiveDevice_001
 * @tc.number: RemoteOffloadAudioRenderSink_UpdateActiveDevice_001
 * @tc.desc  : Test UpdateActiveDevice on uninitialized sink, should return NOT_SUPPORTED
 */
AIUT_F(RemoteOffloadAudioRenderSinkAiut, RemoteOffloadAudioRenderSink_UpdateActiveDevice_001)
{
    AIUT_INFO_LOG("RemoteOffloadAudioRenderSink_UpdateActiveDevice_001 start");
    ASSERT_NE(sink_, nullptr);
    std::vector<DeviceType> deviceTypes = {DEVICE_TYPE_SPEAKER};
    int32_t ret = sink_->UpdateActiveDevice(deviceTypes);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

} // namespace AudioStandard
} // namespace OHOS
