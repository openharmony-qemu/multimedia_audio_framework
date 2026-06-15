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

#include "remote_multichannel_audio_render_sink_aiut.h"
#include "aiut_log.h"

using namespace testing::ext;
using namespace OHOS::HDI::DistributedAudio::Audio::V2_0;

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr const char *DEVICE_NETWORK_ID = "test_network_id";
constexpr float TEST_VOLUME_LEFT = 1.0f;
constexpr float TEST_VOLUME_RIGHT = 1.0f;
constexpr float TEST_VOLUME_ZERO = 0.0f;
constexpr size_t TEST_DATA_SIZE = 1024;
}

void RemoteMultichannelAudioRenderSinkAiut::SetUpTestCase(void) {}
void RemoteMultichannelAudioRenderSinkAiut::TearDownTestCase(void) {}

void RemoteMultichannelAudioRenderSinkAiut::SetUp(void)
{
    sink_ = std::make_shared<RemoteMultichannelAudioRenderSink>(DEVICE_NETWORK_ID);
    sink_->sinkInited_.store(true);
    sink_->renderInited_.store(true);
    sink_->validState_.store(true);
    attr_.adapterName = "remote_mch_test";
    attr_.channel = RemoteMultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    attr_.sampleRate = RemoteMultichannelAudioRenderSink::AUDIO_SAMPLE_RATE_48K;
    attr_.format = SAMPLE_S16LE;
}

void RemoteMultichannelAudioRenderSinkAiut::TearDown(void)
{
    if (sink_ && sink_->IsInited()) {
        sink_->DeInit();
    }
    sink_ = nullptr;
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_SetVolume_LeftZero_001
 * @tc.number: RemoteMultichannelAudioRenderSink_SetVolume_LeftZero_001
 * @tc.desc  : Test SetVolume with left volume zero
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut, RemoteMultichannelAudioRenderSink_SetVolume_LeftZero_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_SetVolume_LeftZero_001 start");
    ASSERT_NE(sink_, nullptr);
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, SetVolume(TEST_VOLUME_RIGHT)).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    int32_t ret = sink_->SetVolume(TEST_VOLUME_ZERO, TEST_VOLUME_RIGHT);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(sink_->leftVolume_, TEST_VOLUME_ZERO);
    EXPECT_EQ(sink_->rightVolume_, TEST_VOLUME_RIGHT);
    sink_->audioRender_ = nullptr;
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_SetVolume_RightZero_001
 * @tc.number: RemoteMultichannelAudioRenderSink_SetVolume_RightZero_001
 * @tc.desc  : Test SetVolume with right volume zero
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut, RemoteMultichannelAudioRenderSink_SetVolume_RightZero_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_SetVolume_RightZero_001 start");
    ASSERT_NE(sink_, nullptr);
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, SetVolume(TEST_VOLUME_LEFT)).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    int32_t ret = sink_->SetVolume(TEST_VOLUME_LEFT, TEST_VOLUME_ZERO);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(sink_->leftVolume_, TEST_VOLUME_LEFT);
    EXPECT_EQ(sink_->rightVolume_, TEST_VOLUME_ZERO);
    sink_->audioRender_ = nullptr;
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_SetVolume_BothNonZero_001
 * @tc.number: RemoteMultichannelAudioRenderSink_SetVolume_BothNonZero_001
 * @tc.desc  : Test SetVolume with both non-zero volumes
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut, RemoteMultichannelAudioRenderSink_SetVolume_BothNonZero_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_SetVolume_BothNonZero_001 start");
    ASSERT_NE(sink_, nullptr);
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    float expectedAvgVolume = 1.0f;
    EXPECT_CALL(*mockRender, SetVolume(expectedAvgVolume)).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    int32_t ret = sink_->SetVolume(TEST_VOLUME_LEFT, TEST_VOLUME_RIGHT);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(sink_->leftVolume_, TEST_VOLUME_LEFT);
    EXPECT_EQ(sink_->rightVolume_, TEST_VOLUME_RIGHT);
    sink_->audioRender_ = nullptr;
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_RenderFrame_SwitchDeviceMute_001
 * @tc.number: RemoteMultichannelAudioRenderSink_RenderFrame_SwitchDeviceMute_001
 * @tc.desc  : Test RenderFrame with switchDeviceMute true, data should be zeroed
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut, RemoteMultichannelAudioRenderSink_RenderFrame_SwitchDeviceMute_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_RenderFrame_SwitchDeviceMute_001 start");
    ASSERT_NE(sink_, nullptr);
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, RenderFrame(testing::_, testing::_)).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    sink_->attr_.channel = RemoteMultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->switchDeviceMute_ = true;
    char data[TEST_DATA_SIZE];
    for (size_t i = 0; i < TEST_DATA_SIZE; i++) {
        data[i] = static_cast<char>(0xFF);
    }
    uint64_t writeLen = 0;
    int32_t ret = sink_->RenderFrame(*data, TEST_DATA_SIZE, writeLen);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(data[0], 0);
    sink_->audioRender_ = nullptr;
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_RenderFrame_NoMute_001
 * @tc.number: RemoteMultichannelAudioRenderSink_RenderFrame_NoMute_001
 * @tc.desc  : Test RenderFrame without switchDeviceMute, data should not be zeroed
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut, RemoteMultichannelAudioRenderSink_RenderFrame_NoMute_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_RenderFrame_NoMute_001 start");
    ASSERT_NE(sink_, nullptr);
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, RenderFrame(testing::_, testing::_)).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    sink_->attr_.channel = RemoteMultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->switchDeviceMute_ = false;
    char data[TEST_DATA_SIZE];
    for (size_t i = 0; i < TEST_DATA_SIZE; i++) {
        data[i] = static_cast<char>(0xFF);
    }
    uint64_t writeLen = 0;
    int32_t ret = sink_->RenderFrame(*data, TEST_DATA_SIZE, writeLen);
    EXPECT_EQ(ret, SUCCESS);
    sink_->audioRender_ = nullptr;
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_Stop_AlreadyStopped_001
 * @tc.number: RemoteMultichannelAudioRenderSink_Stop_AlreadyStopped_001
 * @tc.desc  : Test Stop when already stopped
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut, RemoteMultichannelAudioRenderSink_Stop_AlreadyStopped_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_Stop_AlreadyStopped_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->started_.store(false);
    int32_t ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_Stop_NormalStop_001
 * @tc.number: RemoteMultichannelAudioRenderSink_Stop_NormalStop_001
 * @tc.desc  : Test Stop when started with mock render
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut, RemoteMultichannelAudioRenderSink_Stop_NormalStop_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_Stop_NormalStop_001 start");
    ASSERT_NE(sink_, nullptr);
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, Stop()).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    sink_->started_.store(true);
    sink_->validState_.store(true);
    int32_t ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    sink_->audioRender_ = nullptr;
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_UpdateActiveDevice_SameDevice_001
 * @tc.number: RemoteMultichannelAudioRenderSink_UpdateActiveDevice_SameDevice_001
 * @tc.desc  : Test UpdateActiveDevice with same current device
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut, RemoteMultichannelAudioRenderSink_UpdateActiveDevice_SameDevice_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_UpdateActiveDevice_SameDevice_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->currentActiveDevice_ = DEVICE_TYPE_SPEAKER;
    std::vector<DeviceType> deviceTypes = {DEVICE_TYPE_SPEAKER};
    int32_t ret = sink_->UpdateActiveDevice(deviceTypes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_ResetActiveDeviceForDisconnect_001
 * @tc.number: RemoteMultichannelAudioRenderSink_ResetActiveDeviceForDisconnect_001
 * @tc.desc  : Test ResetActiveDeviceForDisconnect with same device
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut,
    RemoteMultichannelAudioRenderSink_ResetActiveDeviceForDisconnect_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_ResetActiveDeviceForDisconnect_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->currentActiveDevice_ = DEVICE_TYPE_SPEAKER;
    sink_->ResetActiveDeviceForDisconnect(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(sink_->currentActiveDevice_, DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_PcmFormatToBit_001
 * @tc.number: RemoteMultichannelAudioRenderSink_PcmFormatToBit_001
 * @tc.desc  : Test PcmFormatToBit static method
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut, RemoteMultichannelAudioRenderSink_PcmFormatToBit_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_PcmFormatToBit_001 start");
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_U8), PCM_8_BIT);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_S16LE), PCM_16_BIT);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_S24LE), PCM_24_BIT);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_S32LE), PCM_32_BIT);
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_SetSinkMuteForSwitchDevice_001
 * @tc.number: RemoteMultichannelAudioRenderSink_SetSinkMuteForSwitchDevice_001
 * @tc.desc  : Test SetSinkMuteForSwitchDevice mute/unmute transitions
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut, RemoteMultichannelAudioRenderSink_SetSinkMuteForSwitchDevice_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_SetSinkMuteForSwitchDevice_001 start");
    auto remoteMchSink = std::make_shared<RemoteMultichannelAudioRenderSink>(DEVICE_NETWORK_ID);
    ASSERT_NE(remoteMchSink, nullptr);

    int32_t ret = remoteMchSink->SetSinkMuteForSwitchDevice(true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(remoteMchSink->muteCount_, 1);
    EXPECT_TRUE(remoteMchSink->switchDeviceMute_.load());

    ret = remoteMchSink->SetSinkMuteForSwitchDevice(true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(remoteMchSink->muteCount_, 2);

    ret = remoteMchSink->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(remoteMchSink->muteCount_, 1);

    ret = remoteMchSink->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(remoteMchSink->muteCount_, 0);
    EXPECT_FALSE(remoteMchSink->switchDeviceMute_.load());
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_OnAudioParamChange_ParamKeyState_001
 * @tc.number: RemoteMultichannelAudioRenderSink_OnAudioParamChange_ParamKeyState_001
 * @tc.desc  : Test OnAudioParamChange with PARAM_KEY_STATE
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut,
    RemoteMultichannelAudioRenderSink_OnAudioParamChange_ParamKeyState_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_OnAudioParamChange_ParamKeyState_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->sinkInited_.store(true);
    sink_->started_.store(true);
    sink_->OnAudioParamChange("adapter", AudioParamKey::PARAM_KEY_STATE, "condition", "value");
    EXPECT_FALSE(sink_->sinkInited_.load());
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_OnAudioParamChange_OtherKey_001
 * @tc.number: RemoteMultichannelAudioRenderSink_OnAudioParamChange_OtherKey_001
 * @tc.desc  : Test OnAudioParamChange with other key
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut,
    RemoteMultichannelAudioRenderSink_OnAudioParamChange_OtherKey_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_OnAudioParamChange_OtherKey_001 start");
    ASSERT_NE(sink_, nullptr);
    sink_->sinkInited_.store(true);
    sink_->started_.store(true);
    sink_->OnAudioParamChange("adapter", AudioParamKey::NONE, "condition", "value");
    EXPECT_TRUE(sink_->sinkInited_.load());
}

/**
 * @tc.name  : RemoteMultichannelAudioRenderSink_GetAudioCategory_001
 * @tc.number: RemoteMultichannelAudioRenderSink_GetAudioCategory_001
 * @tc.desc  : Test GetAudioCategory static method
 */
AIUT_F(RemoteMultichannelAudioRenderSinkAiut, RemoteMultichannelAudioRenderSink_GetAudioCategory_001)
{
    AIUT_INFO_LOG("RemoteMultichannelAudioRenderSink_GetAudioCategory_001 start");
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_DEFAULT), AUDIO_IN_MEDIA);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_RINGING), AUDIO_IN_RINGTONE);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_PHONE_CALL), AUDIO_IN_CALL);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_PHONE_CHAT), AUDIO_IN_COMMUNICATION);
}

} // namespace AudioStandard
} // namespace OHOS
