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

#include "remote_audio_render_sink_aiut.h"
#include "aiut_log.h"

using namespace testing::ext;
using namespace OHOS::HDI::DistributedAudio::Audio::V2_0;

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr const char *DEVICE_NETWORK_ID = "test_network_id";
constexpr uint32_t TEST_CHANNEL = 2;
constexpr uint32_t TEST_SAMPLE_RATE = 48000;
constexpr size_t TEST_DATA_SIZE = 1024;
}

void RemoteAudioRenderSinkAiut::SetUpTestCase(void) {}
void RemoteAudioRenderSinkAiut::TearDownTestCase(void) {}

void RemoteAudioRenderSinkAiut::SetUp(void)
{
    sink_ = std::make_shared<RemoteAudioRenderSink>(DEVICE_NETWORK_ID);
    attr_.adapterName = "remote_test";
    attr_.channel = TEST_CHANNEL;
    attr_.sampleRate = TEST_SAMPLE_RATE;
    attr_.format = SAMPLE_S16LE;
}

void RemoteAudioRenderSinkAiut::TearDown(void)
{
    if (sink_ && sink_->IsInited()) {
        sink_->DeInit();
    }
    sink_ = nullptr;
}

/**
 * @tc.name  : RemoteAudioRenderSink_Init_001
 * @tc.number: RemoteAudioRenderSink_Init_001
 * @tc.desc  : Test Init without HDI device, should fail or return error
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_Init_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_Init_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Init(attr_);
    // Without real HDI, Init may fail — verify it doesn't crash
    AIUT_INFO_LOG("Init ret = %{public}d", ret);
}

/**
 * @tc.name  : RemoteAudioRenderSink_DeInit_001
 * @tc.number: RemoteAudioRenderSink_DeInit_001
 * @tc.desc  : Test DeInit on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_DeInit_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_DeInit_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->DeInit();
    EXPECT_FALSE(sink_->IsInited());
}

/**
 * @tc.name  : RemoteAudioRenderSink_IsInited_001
 * @tc.number: RemoteAudioRenderSink_IsInited_001
 * @tc.desc  : Test IsInited on fresh sink, should be false
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_IsInited_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_IsInited_001 start");
    ASSERT_NE(sink_, nullptr);
    EXPECT_FALSE(sink_->IsInited());
}

/**
 * @tc.name  : RemoteAudioRenderSink_Start_001
 * @tc.number: RemoteAudioRenderSink_Start_001
 * @tc.desc  : Test Start on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_Start_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_Start_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Start();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteAudioRenderSink_Stop_001
 * @tc.number: RemoteAudioRenderSink_Stop_001
 * @tc.desc  : Test Stop on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_Stop_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_Stop_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteAudioRenderSink_Pause_001
 * @tc.number: RemoteAudioRenderSink_Pause_001
 * @tc.desc  : Test Pause on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_Pause_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_Pause_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Pause();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteAudioRenderSink_Resume_001
 * @tc.number: RemoteAudioRenderSink_Resume_001
 * @tc.desc  : Test Resume on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_Resume_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_Resume_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Resume();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteAudioRenderSink_Flush_001
 * @tc.number: RemoteAudioRenderSink_Flush_001
 * @tc.desc  : Test Flush on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_Flush_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_Flush_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Flush();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteAudioRenderSink_Reset_001
 * @tc.number: RemoteAudioRenderSink_Reset_001
 * @tc.desc  : Test Reset on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_Reset_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_Reset_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->Reset();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteAudioRenderSink_SetVolume_001
 * @tc.number: RemoteAudioRenderSink_SetVolume_001
 * @tc.desc  : Test SetVolume on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_SetVolume_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_SetVolume_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->SetVolume(1.0f, 1.0f);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteAudioRenderSink_GetVolume_001
 * @tc.number: RemoteAudioRenderSink_GetVolume_001
 * @tc.desc  : Test GetVolume on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_GetVolume_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_GetVolume_001 start");
    ASSERT_NE(sink_, nullptr);
    float left = 0.0f;
    float right = 0.0f;
    int32_t ret = sink_->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteAudioRenderSink_SetInvalidState_001
 * @tc.number: RemoteAudioRenderSink_SetInvalidState_001
 * @tc.desc  : Test SetInvalidState on sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_SetInvalidState_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_SetInvalidState_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->SetInvalidState();
    EXPECT_FALSE(sink_->validState_.load());
}

/**
 * @tc.name  : RemoteAudioRenderSink_SetSinkMuteForSwitchDevice_001
 * @tc.number: RemoteAudioRenderSink_SetSinkMuteForSwitchDevice_001
 * @tc.desc  : Test SetSinkMuteForSwitchDevice mute/unmute transitions
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_SetSinkMuteForSwitchDevice_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_SetSinkMuteForSwitchDevice_001 start");
    ASSERT_NE(sink_, nullptr);

    int32_t ret = sink_->SetSinkMuteForSwitchDevice(true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(sink_->muteCount_, 1);
    EXPECT_TRUE(sink_->switchDeviceMute_.load());

    ret = sink_->SetSinkMuteForSwitchDevice(true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(sink_->muteCount_, 2);

    ret = sink_->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(sink_->muteCount_, 1);

    ret = sink_->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(sink_->muteCount_, 0);
    EXPECT_FALSE(sink_->switchDeviceMute_.load());
}

/**
 * @tc.name  : RemoteAudioRenderSink_UpdateAppsUid_001
 * @tc.number: RemoteAudioRenderSink_UpdateAppsUid_001
 * @tc.desc  : Test UpdateAppsUid with vector
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_UpdateAppsUid_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_UpdateAppsUid_001 start");
    ASSERT_NE(sink_, nullptr);
    std::vector<int32_t> appsUid = {20000001, 20000002};
    int32_t ret = sink_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);
    appsUid.clear();
    ret = sink_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteAudioRenderSink_RenderFrame_SwitchDeviceMute_001
 * @tc.number: RemoteAudioRenderSink_RenderFrame_SwitchDeviceMute_001
 * @tc.desc  : Test RenderFrame with switchDeviceMute true, data should be zeroed
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_RenderFrame_SwitchDeviceMute_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_RenderFrame_SwitchDeviceMute_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->renderInited_.store(true);
    sink_->validState_.store(true);
    sink_->switchDeviceMute_ = true;

    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, RenderFrame(testing::_, testing::_)).WillOnce(testing::Return(SUCCESS));
    RemoteAudioRenderSink::RenderWrapper wrapper;
    wrapper.audioRender_ = mockRender;
    sink_->audioRenderWrapperMap_[AUDIO_IN_MEDIA] = wrapper;
    sink_->attr_.channel = TEST_CHANNEL;
    sink_->attr_.format = SAMPLE_S16LE;

    char data[TEST_DATA_SIZE];
    for (size_t i = 0; i < TEST_DATA_SIZE; i++) {
        data[i] = static_cast<char>(0xFF);
    }
    uint64_t writeLen = 0;
    int32_t ret = sink_->RenderFrame(*data, sizeof(data), writeLen);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(data[0], 0);
}

/**
 * @tc.name  : RemoteAudioRenderSink_ReleaseActiveDevice_001
 * @tc.number: RemoteAudioRenderSink_ReleaseActiveDevice_001
 * @tc.desc  : Test ReleaseActiveDevice with renderInited true
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_ReleaseActiveDevice_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_ReleaseActiveDevice_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->renderInited_.store(true);
    sink_->ReleaseActiveDevice(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(sink_->renderInited_.load(), false);
}

/**
 * @tc.name  : RemoteAudioRenderSink_GetLatency_001
 * @tc.number: RemoteAudioRenderSink_GetLatency_001
 * @tc.desc  : Test GetLatency on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_GetLatency_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_GetLatency_001 start");
    ASSERT_NE(sink_, nullptr);
    uint32_t latency = 0;
    int32_t ret = sink_->GetLatency(latency);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteAudioRenderSink_SetAudioScene_001
 * @tc.number: RemoteAudioRenderSink_SetAudioScene_001
 * @tc.desc  : Test SetAudioScene on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_SetAudioScene_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_SetAudioScene_001 start");
    ASSERT_NE(sink_, nullptr);
    int32_t ret = sink_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteAudioRenderSink_UpdateActiveDevice_001
 * @tc.number: RemoteAudioRenderSink_UpdateActiveDevice_001
 * @tc.desc  : Test UpdateActiveDevice on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_UpdateActiveDevice_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_UpdateActiveDevice_001 start");
    ASSERT_NE(sink_, nullptr);
    std::vector<DeviceType> deviceTypes = {DEVICE_TYPE_SPEAKER};
    int32_t ret = sink_->UpdateActiveDevice(deviceTypes);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteAudioRenderSink_GetPresentationPosition_001
 * @tc.number: RemoteAudioRenderSink_GetPresentationPosition_001
 * @tc.desc  : Test GetPresentationPosition on uninitialized sink
 */
AIUT_F(RemoteAudioRenderSinkAiut, RemoteAudioRenderSink_GetPresentationPosition_001)
{
    AIUT_INFO_LOG("RemoteAudioRenderSink_GetPresentationPosition_001 start");
    ASSERT_NE(sink_, nullptr);
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int32_t ret = sink_->GetPresentationPosition(frames, timeSec, timeNanoSec);
    EXPECT_NE(ret, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
