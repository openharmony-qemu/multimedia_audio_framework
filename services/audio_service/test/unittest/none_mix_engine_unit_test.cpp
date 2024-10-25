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
    noneMixEngineRet.device_.deviceType = DEVICE_TYPE_NONE;
    noneMixEngineRet.renderSink_ = nullptr;

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
    noneMixEngineRet.device_.deviceType = DEVICE_TYPE_INVALID;
    noneMixEngineRet.renderSink_ = nullptr;

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
 * @tc.number: NoneMixEngine_009
 * @tc.desc  : Test NoneMixEngine interface.
 */
HWTEST_F(NoneMixEngineUnitTest, NoneMixEngine_009, TestSize.Level1)
{
    NoneMixEngine noneMixEngineRet;
    AudioSampleFormat formatRet = AudioSampleFormat::SAMPLE_S32LE;
    auto ret = noneMixEngineRet.GetDirectDeviceFormate(formatRet);
    EXPECT_EQ(ret, HdiAdapterFormat::SAMPLE_S32);

    formatRet = AudioSampleFormat::SAMPLE_F32LE;
    ret = noneMixEngineRet.GetDirectDeviceFormate(formatRet);
    EXPECT_EQ(ret, HdiAdapterFormat::SAMPLE_F32);

    formatRet = AudioSampleFormat::INVALID_WIDTH;
    ret = noneMixEngineRet.GetDirectDeviceFormate(formatRet);
    EXPECT_EQ(ret, HdiAdapterFormat::SAMPLE_S16);
}
} // namespace AudioStandard
} // namespace OHOS
