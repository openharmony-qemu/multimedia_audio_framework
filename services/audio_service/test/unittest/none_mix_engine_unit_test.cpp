/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include "none_mix_engine.h"
#include "sink/i_audio_render_sink.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "pro_renderer_stream_impl.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
constexpr int32_t DEFAULT_STREAM_ID = 10;
class NoneMixEngineUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    AudioProcessConfig InitProcessConfig();

protected:
    std::unique_ptr<AudioPlaybackEngine> playbackEngine_;
};

void NoneMixEngineUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void NoneMixEngineUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void NoneMixEngineUnitTest::SetUp(void)
{
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    playbackEngine_ = std::make_unique<NoneMixEngine>();
    playbackEngine_->Init(deviceInfo, false);
}

void NoneMixEngineUnitTest::TearDown(void)
{
    if (playbackEngine_) {
        playbackEngine_->Stop();
        playbackEngine_ = nullptr;
    }
}

AudioProcessConfig NoneMixEngineUnitTest::InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

/**
 * @tc.name  : Test Direct Audio Playback Engine State
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineState_001
 * @tc.desc  : Test direct audio playback engine state(start->pause->flush->stop->release) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineState_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Pause();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Flush();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine State
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineState_002
 * @tc.desc  : Test direct audio playback engine state init success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineState_002, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);

    // ERR_ILLEGAL_STATE
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->InitParams();
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);

    // ERR_ILLEGAL_STATE
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->InitParams();
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);

    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine State
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineState_003
 * @tc.desc  : Test direct audio playback engine state start success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineState_003, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);

    // ERR_ILLEGAL_STATE
    ret = rendererStream->Start();
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine State
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineState_004
 * @tc.desc  : Test direct audio playback engine state pause success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineState_004, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Pause();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine State
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineState_005
 * @tc.desc  : Test direct audio playback engine state flush success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineState_005, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Flush();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine State
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineState_006
 * @tc.desc  : Test direct audio playback engine state drain success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineState_006, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Drain();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine Set Config
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineSetConfig_001
 * @tc.desc  : Test direct audio playback engine set config (sampleRate 192000) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineSetConfig_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_192000;
    config.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine Set Config
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineSetConfig_002
 * @tc.desc  : Test direct audio playback engine set config (deviceType DEVICE_TYPE_USB_HEADSET) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineSetConfig_002, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_192000;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine Set Config
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineSetConfig_003
 * @tc.desc  : Test direct audio playback engine set config (sampleRate 176400) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineSetConfig_003, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_176400;
    config.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine Set Config
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineSetConfig_004
 * @tc.desc  : Test direct audio playback engine set config (deviceType DEVICE_TYPE_USB_HEADSET) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineSetConfig_004, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_176400;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine Set Config
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineSetConfig_005
 * @tc.desc  : Test direct audio playback engine set config (sampleRate 96000) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineSetConfig_005, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    config.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine Set Config
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineSetConfig_006
 * @tc.desc  : Test direct audio playback engine set config (deviceType DEVICE_TYPE_USB_HEADSET) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineSetConfig_006, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine Set Config
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineSetConfig_007
 * @tc.desc  : Test direct audio playback engine set config (sampleRate 88200) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineSetConfig_007, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_88200;
    config.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine Set Config
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineSetConfig_008
 * @tc.desc  : Test direct audio playback engine set config (deviceType DEVICE_TYPE_USB_HEADSET) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineSetConfig_008, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_88200;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine Set Config
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineSetConfig_009
 * @tc.desc  : Test direct audio playback engine set config (sampleRate 48000) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineSetConfig_009, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    config.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine Set Config
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineSetConfig_010
 * @tc.desc  : Test direct audio playback engine set config (deviceType DEVICE_TYPE_USB_HEADSET) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineSetConfig_010, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine Set Config
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineSetConfig_011
 * @tc.desc  : Test direct audio playback engine set config (sampleRate 44100) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineSetConfig_011, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    config.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Direct Audio Playback Engine Set Config
 * @tc.type  : FUNC
 * @tc.number: DirectAudioPlayBackEngineSetConfig_012
 * @tc.desc  : Test direct audio playback engine set config (deviceType DEVICE_TYPE_USB_HEADSET) success
 */
HWTEST_F(NoneMixEngineUnitTest, DirectAudioPlayBackEngineSetConfig_012, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t ret = rendererStream->InitParams();
    EXPECT_EQ(SUCCESS, ret);
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    ret = rendererStream->Start();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Stop();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererStream->Release();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_001
 * @tc.desc  : Test NoneMixEngine interface.
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_001, TestSize.Level1)
{
    NoneMixEngine noneMixEngineRet;
    AudioDeviceDescriptor deviceRet(AudioDeviceDescriptor::DEVICE_INFO);
    bool isVoipRet = true;
    noneMixEngineRet.isInit_ = true;
    deviceRet.deviceType_ = DEVICE_TYPE_INVALID;
    noneMixEngineRet.device_.deviceType_ = DEVICE_TYPE_NONE;
    noneMixEngineRet.renderId_ = HDI_INVALID_ID;

    auto ret = noneMixEngineRet.Init(deviceRet, isVoipRet);
    EXPECT_EQ(ret, SUCCESS);

    ret = noneMixEngineRet.Init(deviceRet, isVoipRet);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_002
 * @tc.desc  : Test NoneMixEngine interface.
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_002, TestSize.Level1)
{
    NoneMixEngine noneMixEngineRet;
    AudioDeviceDescriptor deviceRet(AudioDeviceDescriptor::DEVICE_INFO);
    bool isVoipRet = true;
    noneMixEngineRet.isInit_ = true;
    deviceRet.deviceType_ = DEVICE_TYPE_INVALID;
    noneMixEngineRet.device_.deviceType_ = DEVICE_TYPE_INVALID;
    noneMixEngineRet.renderId_ = HDI_INVALID_ID;

    auto ret = noneMixEngineRet.Init(deviceRet, isVoipRet);
    EXPECT_EQ(ret, SUCCESS);

    deviceRet.deviceType_ = DEVICE_TYPE_NONE;
    ret = noneMixEngineRet.Init(deviceRet, isVoipRet);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_003
 * @tc.desc  : Test NoneMixEngine interface.
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_003, TestSize.Level1)
{
    NoneMixEngine noneMixEngineRet;
    noneMixEngineRet.playbackThread_ = nullptr;
    noneMixEngineRet.isStart_ = true;

    auto ret = noneMixEngineRet.Stop();
    EXPECT_EQ(ret, SUCCESS);

    noneMixEngineRet.PauseAsync();
    ret = noneMixEngineRet.StopAudioSink();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_004
 * @tc.desc  : Test NoneMixEngine interface.
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_004, TestSize.Level1)
{
    NoneMixEngine noneMixEngineRet;
    noneMixEngineRet.playbackThread_ = nullptr;
    noneMixEngineRet.isStart_ = true;

    noneMixEngineRet.Flush();
    noneMixEngineRet.MixStreams();
    auto ret = noneMixEngineRet.Pause();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_005
 * @tc.desc  : Test NoneMixEngine interface.
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_005, TestSize.Level1)
{
    NoneMixEngine noneMixEngineRet;
    noneMixEngineRet.playbackThread_ = nullptr;
    noneMixEngineRet.isStart_ = true;
    uint32_t indexRet1 = 0;
    uint32_t indexRet2 = 1;
    AudioProcessConfig configRet;
    std::shared_ptr<ProRendererStreamImpl> rendererStream1 = std::make_shared<ProRendererStreamImpl>(configRet, true);
    rendererStream1->SetStreamIndex(indexRet1);
    noneMixEngineRet.stream_ = rendererStream1;

    auto ret = noneMixEngineRet.AddRenderer(rendererStream1);
    EXPECT_EQ(ret, SUCCESS);

    std::shared_ptr<ProRendererStreamImpl> rendererStream2 = std::make_shared<ProRendererStreamImpl>(configRet, true);
    rendererStream2->SetStreamIndex(indexRet2);
    ret = noneMixEngineRet.AddRenderer(rendererStream2);
    EXPECT_EQ(ret, ERROR_UNSUPPORTED);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_006
 * @tc.desc  : Test NoneMixEngine interface.
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_006, TestSize.Level1)
{
    NoneMixEngine noneMixEngineRet;
    noneMixEngineRet.playbackThread_ = nullptr;
    noneMixEngineRet.isStart_ = true;
    uint32_t indexRet1 = 0;
    uint32_t indexRet2 = 1;
    AudioProcessConfig configRet;
    std::shared_ptr<ProRendererStreamImpl> rendererStream1 = std::make_shared<ProRendererStreamImpl>(configRet, true);
    rendererStream1->SetStreamIndex(indexRet1);

    EXPECT_EQ(noneMixEngineRet.stream_, nullptr);
    noneMixEngineRet.RemoveRenderer(rendererStream1);
    noneMixEngineRet.stream_ = rendererStream1;

    std::shared_ptr<ProRendererStreamImpl> rendererStream2 = std::make_shared<ProRendererStreamImpl>(configRet, true);
    rendererStream2->SetStreamIndex(indexRet2);
    EXPECT_NE(noneMixEngineRet.stream_->GetStreamIndex(), rendererStream2->GetStreamIndex());
    noneMixEngineRet.RemoveRenderer(rendererStream2);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_007
 * @tc.desc  : Test NoneMixEngine interface.
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_007, TestSize.Level1)
{
    NoneMixEngine noneMixEngineRet;
    AudioSamplingRate sampleRateRet = AudioSamplingRate::SAMPLE_RATE_44100;
    auto ret = noneMixEngineRet.GetDirectSampleRate(sampleRateRet);
    EXPECT_EQ(ret, AudioSamplingRate::SAMPLE_RATE_48000);

    sampleRateRet = AudioSamplingRate::SAMPLE_RATE_88200;
    ret = noneMixEngineRet.GetDirectSampleRate(sampleRateRet);
    EXPECT_EQ(ret, AudioSamplingRate::SAMPLE_RATE_96000);

    sampleRateRet = AudioSamplingRate::SAMPLE_RATE_176400;
    ret = noneMixEngineRet.GetDirectSampleRate(sampleRateRet);
    EXPECT_EQ(ret, AudioSamplingRate::SAMPLE_RATE_192000);

    sampleRateRet = AudioSamplingRate::SAMPLE_RATE_8000;
    ret = noneMixEngineRet.GetDirectSampleRate(sampleRateRet);
    EXPECT_EQ(ret, AudioSamplingRate::SAMPLE_RATE_8000);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_008
 * @tc.desc  : Test NoneMixEngine interface.
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_008, TestSize.Level1)
{
    NoneMixEngine noneMixEngineRet;
    AudioSamplingRate sampleRateRet = AudioSamplingRate::SAMPLE_RATE_8000;
    auto ret = noneMixEngineRet.GetDirectVoipSampleRate(sampleRateRet);
    EXPECT_EQ(ret, AudioSamplingRate::SAMPLE_RATE_16000);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_010
 * @tc.desc  : Test NoneMixEngine::Start()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_010, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    auto ret = ptrNoneMixEngine->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_011
 * @tc.desc  : Test NoneMixEngine::DoFadeinOut
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_011, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    bool isFadeOut = true;
    char buffer = 'a';
    size_t bufferSize = 10;
    char *pBuffer = &buffer;
    EXPECT_NE(pBuffer, nullptr);

    ptrNoneMixEngine->uChannel_ = 10;
    ptrNoneMixEngine->uFormat_ = sizeof(int16_t);

    ptrNoneMixEngine->DoFadeinOut(isFadeOut, pBuffer, bufferSize);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_012
 * @tc.desc  : Test NoneMixEngine::DoFadeinOut
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_012, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    bool isFadeOut = true;
    char buffer = 'a';
    size_t bufferSize = 10;
    char *pBuffer = &buffer;
    EXPECT_NE(pBuffer, nullptr);

    ptrNoneMixEngine->uChannel_ = 10;
    ptrNoneMixEngine->uFormat_ = sizeof(int32_t);

    ptrNoneMixEngine->DoFadeinOut(isFadeOut, pBuffer, bufferSize);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_013
 * @tc.desc  : Test NoneMixEngine::DoFadeinOut
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_013, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    bool isFadeOut = false;
    char buffer = 'a';
    size_t bufferSize = 10;
    char *pBuffer = &buffer;
    EXPECT_NE(pBuffer, nullptr);

    ptrNoneMixEngine->uChannel_ = 10;
    ptrNoneMixEngine->uFormat_ = sizeof(int16_t);

    ptrNoneMixEngine->DoFadeinOut(isFadeOut, pBuffer, bufferSize);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_014
 * @tc.desc  : Test NoneMixEngine::DoFadeinOut
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_014, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    bool isFadeOut = false;
    char buffer = 'a';
    size_t bufferSize = 10;
    char *pBuffer = &buffer;
    EXPECT_NE(pBuffer, nullptr);

    ptrNoneMixEngine->uChannel_ = 10;
    ptrNoneMixEngine->uFormat_ = sizeof(int32_t);

    ptrNoneMixEngine->DoFadeinOut(isFadeOut, pBuffer, bufferSize);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_015
 * @tc.desc  : Test NoneMixEngine::MixStreams
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_015, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioProcessConfig configRet;

    ptrNoneMixEngine->stream_ = std::make_shared<ProRendererStreamImpl>(configRet, true);
    EXPECT_NE(ptrNoneMixEngine->stream_, nullptr);

    ptrNoneMixEngine->MixStreams();
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_016
 * @tc.desc  : Test NoneMixEngine::MixStreams
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_016, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioProcessConfig configRet;

    ptrNoneMixEngine->stream_ = nullptr;

    ptrNoneMixEngine->MixStreams();
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_017
 * @tc.desc  : Test NoneMixEngine::MixStreams
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_017, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioProcessConfig configRet;

    ptrNoneMixEngine->stream_ = std::make_shared<ProRendererStreamImpl>(configRet, true);
    EXPECT_NE(ptrNoneMixEngine->stream_, nullptr);
    ptrNoneMixEngine->failedCount_.store(100);

    ptrNoneMixEngine->MixStreams();
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_018
 * @tc.desc  : Test NoneMixEngine::MixStreams
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_018, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioProcessConfig configRet;

    ptrNoneMixEngine->stream_ = std::make_shared<ProRendererStreamImpl>(configRet, true);
    EXPECT_NE(ptrNoneMixEngine->stream_, nullptr);
    ptrNoneMixEngine->failedCount_.store(10);
    ptrNoneMixEngine->startFadeout_.store(true);

    ptrNoneMixEngine->MixStreams();
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_019
 * @tc.desc  : Test NoneMixEngine::MixStreams
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_019, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioProcessConfig configRet;

    ptrNoneMixEngine->stream_ = std::make_shared<ProRendererStreamImpl>(configRet, true);
    EXPECT_NE(ptrNoneMixEngine->stream_, nullptr);
    ptrNoneMixEngine->failedCount_.store(10);
    ptrNoneMixEngine->startFadeout_.store(false);
    ptrNoneMixEngine->startFadein_.store(true);

    ptrNoneMixEngine->MixStreams();
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_020
 * @tc.desc  : Test NoneMixEngine::MixStreams
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_020, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioProcessConfig configRet;

    ptrNoneMixEngine->stream_ = std::make_shared<ProRendererStreamImpl>(configRet, true);
    EXPECT_NE(ptrNoneMixEngine->stream_, nullptr);
    ptrNoneMixEngine->failedCount_.store(10);
    ptrNoneMixEngine->startFadeout_.store(false);
    ptrNoneMixEngine->startFadein_.store(false);

    ptrNoneMixEngine->MixStreams();
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_021
 * @tc.desc  : Test NoneMixEngine::GetDirectSampleRate
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_021, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioSamplingRate sampleRateRet = AudioSamplingRate::SAMPLE_RATE_44100;
    auto ret = ptrNoneMixEngine->GetDirectSampleRate(sampleRateRet);
    EXPECT_EQ(ret, AudioSamplingRate::SAMPLE_RATE_48000);

    sampleRateRet = AudioSamplingRate::SAMPLE_RATE_88200;
    ret = ptrNoneMixEngine->GetDirectSampleRate(sampleRateRet);
    EXPECT_EQ(ret, AudioSamplingRate::SAMPLE_RATE_96000);

    sampleRateRet = AudioSamplingRate::SAMPLE_RATE_176400;
    ret = ptrNoneMixEngine->GetDirectSampleRate(sampleRateRet);
    EXPECT_EQ(ret, AudioSamplingRate::SAMPLE_RATE_192000);

    sampleRateRet = AudioSamplingRate::SAMPLE_RATE_8000;
    ret = ptrNoneMixEngine->GetDirectSampleRate(sampleRateRet);
    EXPECT_EQ(ret, AudioSamplingRate::SAMPLE_RATE_8000);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_022
 * @tc.desc  : Test NoneMixEngine::InitSink
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_022, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = false;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    ptrNoneMixEngine->InitSink(streamInfo);
}


/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_023
 * @tc.desc  : Test NoneMixEngine::InitSink
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_023, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = true;
    ptrNoneMixEngine->renderId_ = HDI_INVALID_ID;

    ptrNoneMixEngine->InitSink(streamInfo);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_024
 * @tc.desc  : Test NoneMixEngine::InitSink
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_024, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = true;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    streamInfo.channels = AudioChannel::CHANNEL_15;
    ptrNoneMixEngine->uChannel_ = 1;

    ptrNoneMixEngine->InitSink(streamInfo);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_025
 * @tc.desc  : Test NoneMixEngine::InitSink
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_025, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = true;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    streamInfo.channels = AudioChannel::CHANNEL_15;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    ptrNoneMixEngine->uChannel_ = 2;
    ptrNoneMixEngine->uFormat_ = SAMPLE_S32LE;

    ptrNoneMixEngine->InitSink(streamInfo);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_026
 * @tc.desc  : Test NoneMixEngine::InitSink
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_026, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = true;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    streamInfo.channels = AudioChannel::CHANNEL_15;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    ptrNoneMixEngine->uChannel_ = 2;
    ptrNoneMixEngine->uFormat_ = SAMPLE_S16LE;
    ptrNoneMixEngine->isVoip_ = true;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    ptrNoneMixEngine->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_24000;

    ptrNoneMixEngine->InitSink(streamInfo);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_027
 * @tc.desc  : Test NoneMixEngine::InitSink
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_027, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = true;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    streamInfo.channels = AudioChannel::CHANNEL_15;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    ptrNoneMixEngine->uChannel_ = 2;
    ptrNoneMixEngine->uFormat_ = SAMPLE_S16LE;
    ptrNoneMixEngine->isVoip_ = true;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    ptrNoneMixEngine->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_16000;

    ptrNoneMixEngine->InitSink(streamInfo);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_028
 * @tc.desc  : Test NoneMixEngine::SwitchSink
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_028, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = true;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    streamInfo.channels = AudioChannel::CHANNEL_15;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    ptrNoneMixEngine->uChannel_ = 2;
    ptrNoneMixEngine->uFormat_ = SAMPLE_S16LE;
    ptrNoneMixEngine->isVoip_ = true;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    ptrNoneMixEngine->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_16000;
    bool isVoip = true;
    int32_t ret = ptrNoneMixEngine->SwitchSink(streamInfo, isVoip);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_029
 * @tc.desc  : Test NoneMixEngine::SwitchSink
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_029, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = false;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    streamInfo.channels = AudioChannel::CHANNEL_15;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    ptrNoneMixEngine->uChannel_ = 2;
    ptrNoneMixEngine->uFormat_ = SAMPLE_S16LE;
    ptrNoneMixEngine->isVoip_ = true;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    ptrNoneMixEngine->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_16000;
    bool isVoip = true;
    int32_t ret = ptrNoneMixEngine->SwitchSink(streamInfo, isVoip);
    EXPECT_EQ(ret, SUCCESS);
}
/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_030
 * @tc.desc  : Test NoneMixEngine::Start
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_030, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = true;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    streamInfo.channels = AudioChannel::CHANNEL_15;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    ptrNoneMixEngine->uChannel_ = 2;
    ptrNoneMixEngine->uFormat_ = SAMPLE_S16LE;
    ptrNoneMixEngine->isVoip_ = true;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    ptrNoneMixEngine->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_16000;
    bool isVoip = true;
    ptrNoneMixEngine->SwitchSink(streamInfo, isVoip);
    int32_t ret = ptrNoneMixEngine->Start();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_031
 * @tc.desc  : Test NoneMixEngine::Start
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_031, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = false;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    streamInfo.channels = AudioChannel::CHANNEL_15;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    ptrNoneMixEngine->uChannel_ = 2;
    ptrNoneMixEngine->uFormat_ = SAMPLE_S16LE;
    ptrNoneMixEngine->isVoip_ = true;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    ptrNoneMixEngine->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_16000;
    bool isVoip = true;
    ptrNoneMixEngine->SwitchSink(streamInfo, isVoip);
    int32_t ret = ptrNoneMixEngine->Start();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_032
 * @tc.desc  : Test NoneMixEngine::Pause
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_032, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = true;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    streamInfo.channels = AudioChannel::CHANNEL_15;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    ptrNoneMixEngine->uChannel_ = 2;
    ptrNoneMixEngine->uFormat_ = SAMPLE_S16LE;
    ptrNoneMixEngine->isVoip_ = true;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    ptrNoneMixEngine->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_16000;
    bool isVoip = true;
    ptrNoneMixEngine->SwitchSink(streamInfo, isVoip);
    int32_t ret = ptrNoneMixEngine->Pause();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_033
 * @tc.desc  : Test NoneMixEngine::Pause
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_033, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = false;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    streamInfo.channels = AudioChannel::CHANNEL_15;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    ptrNoneMixEngine->uChannel_ = 2;
    ptrNoneMixEngine->uFormat_ = SAMPLE_S16LE;
    ptrNoneMixEngine->isVoip_ = true;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    ptrNoneMixEngine->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_16000;
    bool isVoip = true;
    ptrNoneMixEngine->SwitchSink(streamInfo, isVoip);
    int32_t ret = ptrNoneMixEngine->Pause();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_034
 * @tc.desc  : Test NoneMixEngine interface.
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_034, TestSize.Level1)
{
    NoneMixEngine noneMixEngineRet;
    AudioSamplingRate sampleRateRet = AudioSamplingRate::SAMPLE_RATE_48000;
    auto ret = noneMixEngineRet.GetDirectVoipSampleRate(sampleRateRet);
    EXPECT_EQ(ret, AudioSamplingRate::SAMPLE_RATE_48000);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_035
 * @tc.desc  : Test NoneMixEngine interface.
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_035, TestSize.Level1)
{
    NoneMixEngine noneMixEngineRet;
    AudioSampleFormat formatRet = SAMPLE_S16LE;
    auto ret = noneMixEngineRet.GetDirectFormatByteSize(formatRet);
    EXPECT_EQ(ret, sizeof(int16_t));

    formatRet = SAMPLE_S32LE;
    ret = noneMixEngineRet.GetDirectFormatByteSize(formatRet);
    EXPECT_EQ(ret, sizeof(int32_t));

    formatRet = SAMPLE_F32LE;
    ret = noneMixEngineRet.GetDirectFormatByteSize(formatRet);
    EXPECT_EQ(ret, sizeof(int32_t));

    formatRet = INVALID_WIDTH;
    ret = noneMixEngineRet.GetDirectFormatByteSize(formatRet);
    EXPECT_EQ(ret, sizeof(int32_t));
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_036
 * @tc.desc  : Test NoneMixEngine::GetLatency
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_036, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);
    uint64_t latency = ptrNoneMixEngine->GetLatency();
    EXPECT_EQ(latency, 0);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = false;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    streamInfo.channels = AudioChannel::CHANNEL_15;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    ptrNoneMixEngine->uChannel_ = 2;
    ptrNoneMixEngine->uFormat_ = SAMPLE_S16LE;
    ptrNoneMixEngine->isVoip_ = true;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    ptrNoneMixEngine->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_16000;
    bool isVoip = true;
    ptrNoneMixEngine->SwitchSink(streamInfo, isVoip);
    int32_t ret = ptrNoneMixEngine->Start();
    EXPECT_EQ(ret, SUCCESS);

    latency = ptrNoneMixEngine->GetLatency();
    EXPECT_NE(latency, 0);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_037
 * @tc.desc  : Test NoneMixEngine::Stop()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_037, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    std::string name = "test";
    ptrNoneMixEngine->isStart_ = true;
    ptrNoneMixEngine->playbackThread_ = std::make_unique<AudioThreadTask>(name);
    auto ret = ptrNoneMixEngine->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_038
 * @tc.desc  : Test NoneMixEngine::PauseAsync()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_038, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    ptrNoneMixEngine->playbackThread_ = nullptr;
    ptrNoneMixEngine->PauseAsync();
    EXPECT_EQ(ptrNoneMixEngine->isStart_, false);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_039
 * @tc.desc  : Test NoneMixEngine::StopAudioSink()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_039, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    ptrNoneMixEngine->renderId_ = -1;
    auto ret = ptrNoneMixEngine->StopAudioSink();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_040
 * @tc.desc  : Test NoneMixEngine::StopAudioSink()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_040, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    auto sink = HdiAdapterManager::GetInstance().GetRenderSink(ptrNoneMixEngine->renderId_, true);
    ASSERT_TRUE(sink != nullptr);

    auto ret = ptrNoneMixEngine->StopAudioSink();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_041
 * @tc.desc  : Test NoneMixEngine::Pause()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_041, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    ptrNoneMixEngine->isStart_ = true;
    std::string name = "test";
    ptrNoneMixEngine->playbackThread_ = std::make_unique<AudioThreadTask>(name);
    auto ret = ptrNoneMixEngine->Pause();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_042
 * @tc.desc  : Test NoneMixEngine::DoFadeinOut()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_042, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    bool isFadeOut = true;
    char buffer = 'a';
    size_t bufferSize = 2;
    char *pBuffer = &buffer;
    EXPECT_NE(pBuffer, nullptr);

    ptrNoneMixEngine->uChannel_ = 1;
    ptrNoneMixEngine->uFormat_ = sizeof(int16_t);

    ptrNoneMixEngine->DoFadeinOut(isFadeOut, pBuffer, bufferSize);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_043
 * @tc.desc  : Test NoneMixEngine::DoFadeinOut()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_043, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    char buffer = 'a';
    char *pBuffer = &buffer;
    ptrNoneMixEngine->uFormat_ = 1;
    ptrNoneMixEngine->uChannel_ = 1;
    ptrNoneMixEngine->uFormat_ = sizeof(int16_t);
    bool isFadeOut = false;
    size_t bufferSize = 2;
    ptrNoneMixEngine->DoFadeinOut(isFadeOut, pBuffer, bufferSize);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_044
 * @tc.desc  : Test NoneMixEngine::DoFadeinOut()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_044, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    char buffer = 'a';
    char *pBuffer = &buffer;
    ptrNoneMixEngine->uFormat_ = 1;
    ptrNoneMixEngine->uChannel_ = 1;
    ptrNoneMixEngine->uFormat_ = sizeof(int16_t);
    bool isFadeOut = false;
    size_t bufferSize = 0;
    ptrNoneMixEngine->DoFadeinOut(isFadeOut, pBuffer, bufferSize);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_045
 * @tc.desc  : Test NoneMixEngine::AdjustVoipVolume()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_045, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    ptrNoneMixEngine->isVoip_ = false;
    ptrNoneMixEngine->AdjustVoipVolume();
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_046
 * @tc.desc  : Test NoneMixEngine::AdjustVoipVolume()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_046, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    AudioProcessConfig processConfig;
    ptrNoneMixEngine->isVoip_ = true;
    ptrNoneMixEngine->firstSetVolume_ = false;
    ptrNoneMixEngine->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    ptrNoneMixEngine->AdjustVoipVolume();
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_047
 * @tc.desc  : Test NoneMixEngine::AdjustVoipVolume()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_047, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    AudioProcessConfig processConfig;
    ptrNoneMixEngine->isVoip_ = true;
    ptrNoneMixEngine->firstSetVolume_ = true;
    ptrNoneMixEngine->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    ptrNoneMixEngine->AdjustVoipVolume();
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_048
 * @tc.desc  : Test NoneMixEngine::AddRenderer()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_048, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    AudioProcessConfig configRet;
    std::shared_ptr<ProRendererStreamImpl> rendererStream1 = std::make_shared<ProRendererStreamImpl>(configRet, true);
    ptrNoneMixEngine->stream_ = nullptr;
    ptrNoneMixEngine->AddRenderer(rendererStream1);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_049
 * @tc.desc  : Test NoneMixEngine::AddRenderer()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_049, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    AudioProcessConfig configRet;
    std::shared_ptr<ProRendererStreamImpl> rendererStream1 = std::make_shared<ProRendererStreamImpl>(configRet, true);
    ptrNoneMixEngine->stream_ = rendererStream1;
    ptrNoneMixEngine->AddRenderer(rendererStream1);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_050
 * @tc.desc  : Test NoneMixEngine::GetLatency()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_050, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    ptrNoneMixEngine->isStart_ = true;
    ptrNoneMixEngine->latency_ = 1;
    auto ret = ptrNoneMixEngine->GetLatency();
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_051
 * @tc.desc  : Test NoneMixEngine::Start
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_051, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    EXPECT_NE(ptrNoneMixEngine, nullptr);

    AudioStreamInfo streamInfo;
    ptrNoneMixEngine->isInit_ = true;
    ptrNoneMixEngine->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(ptrNoneMixEngine->renderId_, HDI_INVALID_ID);

    streamInfo.channels = AudioChannel::CHANNEL_15;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    ptrNoneMixEngine->uChannel_ = 2;
    ptrNoneMixEngine->uFormat_ = SAMPLE_S16LE;
    ptrNoneMixEngine->isVoip_ = true;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    ptrNoneMixEngine->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_16000;
    bool isVoip = true;
    ptrNoneMixEngine->SwitchSink(streamInfo, isVoip);

    std::string name = "noneMixThread";
    ptrNoneMixEngine->playbackThread_ = std::make_unique<AudioThreadTask>(name);
    ptrNoneMixEngine->isStart_ = true;
    int32_t ret = ptrNoneMixEngine->Start();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_052
 * @tc.desc  : Test NoneMixEngine::PauseAsync()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_052, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    std::string name = "noneMixThread";
    ptrNoneMixEngine->isStart_ = true;
    ptrNoneMixEngine->playbackThread_ = std::make_unique<AudioThreadTask>(name);
    ptrNoneMixEngine->playbackThread_->state_.store(AudioThreadTask::RunningState::PAUSING);
    ptrNoneMixEngine->PauseAsync();
    EXPECT_EQ(ptrNoneMixEngine->isStart_, false);

    ptrNoneMixEngine->playbackThread_ = nullptr;
    ptrNoneMixEngine->PauseAsync();
    EXPECT_EQ(ptrNoneMixEngine->isStart_, false);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_053
 * @tc.desc  : Test NoneMixEngine::Pause()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_053, TestSize.Level1)
{
    auto ptrNoneMixEngine = std::make_shared<NoneMixEngine>();
    ASSERT_TRUE(ptrNoneMixEngine != nullptr);

    ptrNoneMixEngine->isStart_ = true;
    auto ret = ptrNoneMixEngine->Pause();
    EXPECT_EQ(ret, SUCCESS);

    std::string name = "test";
    ptrNoneMixEngine->isStart_ = true;
    ptrNoneMixEngine->playbackThread_ = std::make_unique<AudioThreadTask>(name);
    ret = ptrNoneMixEngine->Pause();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NoneMixEngine API
 * @tc.type  : FUNC
 * @tc.number: NoneMixEngine_051
 * @tc.desc  : Test NoneMixEngine::GetDirectVoipDeviceFormat()
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_051, TestSize.Level1)
{
    NoneMixEngine noneMixEngineRet;
    AudioSampleFormat formatRet = AudioSampleFormat::SAMPLE_S24LE;
    auto ret = noneMixEngineRet.GetDirectVoipDeviceFormat(formatRet);
    EXPECT_EQ(SAMPLE_S32LE, ret);
 
    formatRet = AudioSampleFormat::SAMPLE_S32LE;
    ret = noneMixEngineRet.GetDirectVoipDeviceFormat(formatRet);
    EXPECT_EQ(SAMPLE_S32LE, ret);
 
    formatRet = AudioSampleFormat::INVALID_WIDTH;
    ret = noneMixEngineRet.GetDirectVoipDeviceFormat(formatRet);
    EXPECT_EQ(SAMPLE_S16LE, ret);
}
} // namespace AudioStandard
} // namespace OHOS
