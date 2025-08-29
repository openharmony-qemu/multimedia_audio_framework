/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "audio_service.h"
#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_system_manager.h"

#include "audio_process_proxy.h"
#include "audio_process_in_client.h"
#include "fast_audio_stream.h"
#include "audio_endpoint_private.h"
#include "pro_renderer_stream_impl.h"
#include "core_service_handler.h"
#include "audio_workgroup.h"
#include "rtg_interface.h"
#include "concurrent_task_client.h"
#include "audio_resource_service.h"
#include "audio_endpoint.h"


using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
std::shared_ptr<AudioProcessInClient> processClient_;
std::shared_ptr<FastAudioStream> fastAudioStream_;
const int32_t RENDERER_FLAGS = 0;
#ifdef HAS_FEATURE_INNERCAPTURER
const int32_t MEDIA_SERVICE_UID = 1013;
#endif

static const uint32_t NORMAL_ENDPOINT_RELEASE_DELAY_TIME_MS = 3000; // 3s
static const uint32_t A2DP_ENDPOINT_RELEASE_DELAY_TIME = 3000; // 3s
static const uint32_t VOIP_ENDPOINT_RELEASE_DELAY_TIME = 200; // 200ms
static const uint32_t VOIP_REC_ENDPOINT_RELEASE_DELAY_TIME = 60; // 60ms
static const uint32_t A2DP_ENDPOINT_RE_CREATE_RELEASE_DELAY_TIME = 200; // 200ms

class AudioServiceUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class AudioParameterCallbackTest : public AudioParameterCallback {
    virtual void OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
        const std::string& condition, const std::string& value) {}
};

void AudioServiceUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioServiceUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioServiceUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioServiceUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_001
 * @tc.desc  : Test AudioProcessInClientInner interface using unsupported parameters.
 */
HWTEST(AudioServiceUnitTest, AudioProcessInClientInner_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_PLAYBACK;

    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.rendererFlags = RENDERER_FLAGS;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;

    fastAudioStream_ = std::make_shared<FastAudioStream>(config.streamType,
        AUDIO_MODE_PLAYBACK, config.appInfo.appUid);
    processClient_ = AudioProcessInClient::Create(config, fastAudioStream_);
    EXPECT_EQ(processClient_, nullptr);
}
/**
 * @tc.name  : Test RegisterThreadPriorityOnStart API
 * @tc.type  : FUNC
 * @tc.number: RegisterThreadPriorityOnStart_001
 * @tc.desc  : Test RegisterThreadPriorityOnStart interface using unsupported parameters.
 */
HWTEST(AudioServiceUnitTest, RegisterThreadPriorityOnStart_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_PLAYBACK;

    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.rendererFlags = RENDERER_FLAGS;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;
    StateChangeCmdType cmdType = CMD_FROM_CLIENT;
    std::unique_ptr<FastAudioStream> fastAudioStream = std::make_unique<FastAudioStream>(config.streamType,
        AUDIO_MODE_PLAYBACK, config.appInfo.appUid);
    fastAudioStream->RegisterThreadPriorityOnStart(cmdType);
    EXPECT_NE(fastAudioStream, nullptr);

    cmdType = CMD_FROM_SYSTEM;
    fastAudioStream->RegisterThreadPriorityOnStart(cmdType);

    cmdType = static_cast<StateChangeCmdType>(2);
    fastAudioStream->RegisterThreadPriorityOnStart(cmdType);
}
/**
 * @tc.name  : Test RegisterThreadPriorityOnStart API
 * @tc.type  : FUNC
 * @tc.number: RegisterThreadPriorityOnStart_001
 * @tc.desc  : Test RegisterThreadPriorityOnStart interface using unsupported parameters.
 */
HWTEST(AudioServiceUnitTest, StartAudioStream_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_PLAYBACK;

    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.rendererFlags = RENDERER_FLAGS;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;
    StateChangeCmdType cmdType = CMD_FROM_SYSTEM;
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReasonExt::ExtEnum::NEW_DEVICE_AVAILABLE);
    std::unique_ptr<FastAudioStream> fastAudioStream = std::make_unique<FastAudioStream>(config.streamType,
        AUDIO_MODE_PLAYBACK, config.appInfo.appUid);
    int ret = fastAudioStream->StartAudioStream(cmdType, reason);
    EXPECT_EQ(ret, 0);

    cmdType = CMD_FROM_CLIENT;
    ret = fastAudioStream->StartAudioStream(cmdType, reason);
    EXPECT_EQ(ret, 0);

    cmdType = static_cast<StateChangeCmdType>(2);
    fastAudioStream->StartAudioStream(cmdType, reason);
    EXPECT_EQ(ret, 0);
}
/**
 * @tc.name  : Test StopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: StopAudioStream_001
 * @tc.desc  : Test StopAudioStream interface using unsupported parameters.
 */
HWTEST(AudioServiceUnitTest, StopAudioStream_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_PLAYBACK;

    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.rendererFlags = RENDERER_FLAGS;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;
    std::unique_ptr<FastAudioStream> fastAudioStream = std::make_unique<FastAudioStream>(config.streamType,
        AUDIO_MODE_PLAYBACK, config.appInfo.appUid);
    int ret = fastAudioStream->StopAudioStream();
    EXPECT_EQ(ret, 0);
}
/**
 * @tc.name  : Test FetchDeviceForSplitStream API
 * @tc.type  : FUNC
 * @tc.number: FetchDeviceForSplitStream
 * @tc.desc  : Test FetchDeviceForSplitStream interface using unsupported parameters.
 */
HWTEST(AudioServiceUnitTest, FetchDeviceForSplitStream_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_PLAYBACK;

    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.rendererFlags = RENDERER_FLAGS;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;
    std::unique_ptr<FastAudioStream> fastAudioStream = std::make_unique<FastAudioStream>(config.streamType,
        AUDIO_MODE_PLAYBACK, config.appInfo.appUid);
    fastAudioStream->FetchDeviceForSplitStream();
    EXPECT_NE(fastAudioStream, nullptr);
}
/**
 * @tc.name  : Test SetCallbacksWhenRestore API
 * @tc.type  : FUNC
 * @tc.number: SetCallbacksWhenRestore_001
 * @tc.desc  : Test SetCallbacksWhenRestore interface using unsupported parameters.
 */
HWTEST(AudioServiceUnitTest, SetCallbacksWhenRestore_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_PLAYBACK;

    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.rendererFlags = RENDERER_FLAGS;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;
    std::unique_ptr<FastAudioStream> fastAudioStream = std::make_unique<FastAudioStream>(config.streamType,
        AUDIO_MODE_PLAYBACK, config.appInfo.appUid);
    int ret = fastAudioStream->SetCallbacksWhenRestore();
    EXPECT_NE(ret, 0);
}
/**
 * @tc.name  : Test RestoreAudioStream API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioStream_001
 * @tc.desc  : Test RestoreAudioStream interface using unsupported parameters.
 */
HWTEST(AudioServiceUnitTest, RestoreAudioStream_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_PLAYBACK;

    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.rendererFlags = RENDERER_FLAGS;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;
    std::unique_ptr<FastAudioStream> fastAudioStream = std::make_unique<FastAudioStream>(config.streamType,
        AUDIO_MODE_PLAYBACK, config.appInfo.appUid);
    bool needStoreState = true;
    int ret = fastAudioStream->RestoreAudioStream(needStoreState);
    EXPECT_EQ(ret, 0);

    needStoreState = false;
    ret = fastAudioStream->RestoreAudioStream(needStoreState);
    EXPECT_EQ(ret, 0);
}
/**
 * @tc.name  : Test JoincallbackLoop API
 * @tc.type  : FUNC
 * @tc.number: JoincallbackLoop_001
 * @tc.desc  : Test JoincallbackLoop interface using unsupported parameters.
 */
HWTEST(AudioServiceUnitTest, JoinCallbackLoop_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_PLAYBACK;

    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.rendererFlags = RENDERER_FLAGS;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;
    std::unique_ptr<FastAudioStream> fastAudioStream = std::make_unique<FastAudioStream>(config.streamType,
        AUDIO_MODE_PLAYBACK, config.appInfo.appUid);
    fastAudioStream->JoinCallbackLoop();
    EXPECT_NE(fastAudioStream, nullptr);
}
/**
 * @tc.name  : Test SetDefaultoutputDevice API
 * @tc.type  : FUNC
 * @tc.number: SetDefaultoutputDevice_001
 * @tc.desc  : Test SetDefaultoutputDevice interface using unsupported parameters.
 */
HWTEST(AudioServiceUnitTest, SetDefaultOutputDevice_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_PLAYBACK;

    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.rendererFlags = RENDERER_FLAGS;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;
    std::unique_ptr<FastAudioStream> fastAudioStream = std::make_unique<FastAudioStream>(config.streamType,
        AUDIO_MODE_PLAYBACK, config.appInfo.appUid);
    DeviceType expectedDevice = static_cast<DeviceType>(1);
    int ret = fastAudioStream->SetDefaultOutputDevice(expectedDevice);
    EXPECT_NE(ret, 0);

    expectedDevice = static_cast<DeviceType>(2);
    ret = fastAudioStream->SetDefaultOutputDevice(expectedDevice);
    EXPECT_NE(ret, 0);
}
/**
 * @tc.name  : Test PauseAudiStream API
 * @tc.type  : FUNC
 * @tc.number: PauseAudiStream
 * @tc.desc  : Test PauseAudiStream interface using unsupported parameters.
 */
HWTEST(AudioServiceUnitTest, PauseAudioStream_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_PLAYBACK;

    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.rendererFlags = RENDERER_FLAGS;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;
    StateChangeCmdType cmdType = static_cast<StateChangeCmdType>(2);
    std::unique_ptr<FastAudioStream> fastAudioStream = std::make_unique<FastAudioStream>(config.streamType,
        AUDIO_MODE_PLAYBACK, config.appInfo.appUid);
    int ret = fastAudioStream->PauseAudioStream(cmdType);
    EXPECT_EQ(ret, 0);

    cmdType = CMD_FROM_CLIENT;
    ret = fastAudioStream->PauseAudioStream(cmdType);
    EXPECT_EQ(ret, 0);

    cmdType = CMD_FROM_SYSTEM;
    ret = fastAudioStream->PauseAudioStream(cmdType);
    EXPECT_EQ(ret, 0);
}
/**
 * @tc.name  : Test JoinCallbackLoop
 * @tc.number: Audio_Renderer_JoinCallbackLoop_001
 * @tc.desc  : Test JoinCallbackLoop interface
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_JoinCallbackLoop_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_PLAYBACK;

    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.rendererFlags = RENDERER_FLAGS;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;

    std::shared_ptr<FastAudioStream> fastAudioStream = std::make_shared<FastAudioStream>(config.streamType,
        AUDIO_MODE_PLAYBACK, config.appInfo.appUid);
    fastAudioStream->JoinCallbackLoop();
    std::shared_ptr<AudioProcessInClient> processClient = AudioProcessInClient::Create(config, fastAudioStream_);
    fastAudioStream->JoinCallbackLoop();
    EXPECT_EQ(processClient_, nullptr);
}

/**
 * @tc.name  : Test AudioDeviceDescriptor API
 * @tc.type  : FUNC
 * @tc.number: AudioDeviceDescriptor_001
 * @tc.desc  : Test AudioDeviceDescriptor interface.
 */
HWTEST(AudioServiceUnitTest, AudioDeviceDescriptor_001, TestSize.Level1)
{
    DeviceType type = DeviceType::DEVICE_TYPE_SPEAKER;
    DeviceRole role = DeviceRole::OUTPUT_DEVICE;
    int32_t interruptGroupId = 1;
    int32_t volumeGroupId = 1;
    std::string networkId = "LocalDevice";
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor =
        std::make_shared<AudioDeviceDescriptor>(type, role, interruptGroupId, volumeGroupId, networkId);
    EXPECT_NE(audioDeviceDescriptor, nullptr);

    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = type;
    deviceDescriptor.deviceRole_ = role;
    audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(deviceDescriptor);
    EXPECT_NE(audioDeviceDescriptor, nullptr);

    std::string deviceName = "";
    std::string macAddress = "";
    audioDeviceDescriptor->SetDeviceInfo(deviceName, macAddress);

    DeviceStreamInfo audioStreamInfo = {
        SAMPLE_RATE_48000,
        ENCODING_PCM,
        SAMPLE_S16LE,
        CH_LAYOUT_STEREO
    };
    int32_t channelMask = 1;
    audioDeviceDescriptor->SetDeviceCapability({ audioStreamInfo }, channelMask);

    DeviceStreamInfo streamInfo = audioDeviceDescriptor->GetDeviceStreamInfo();
    EXPECT_EQ(streamInfo.channelLayout, audioStreamInfo.channelLayout);
    EXPECT_EQ(streamInfo.encoding, audioStreamInfo.encoding);
    EXPECT_EQ(streamInfo.format, audioStreamInfo.format);
    EXPECT_EQ(streamInfo.samplingRate, audioStreamInfo.samplingRate);
}

/**
 * @tc.name  : Test UpdateMuteControlSet API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceUpdateMuteControlSet_001
 * @tc.desc  : Test UpdateMuteControlSet interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceUpdateMuteControlSet_001, TestSize.Level1)
{
    AudioService::GetInstance()->UpdateMuteControlSet(1, true);
    AudioService::GetInstance()->UpdateMuteControlSet(MAX_STREAMID + 1, true);
    AudioService::GetInstance()->UpdateMuteControlSet(MAX_STREAMID - 1, false);
    AudioService::GetInstance()->UpdateMuteControlSet(MAX_STREAMID - 1, true);
    AudioService::GetInstance()->UpdateMuteControlSet(MAX_STREAMID - 1, false);
    AudioService::GetInstance()->UpdateMuteControlSet(MAX_STREAMID - 1, true);
    AudioService::GetInstance()->RemoveIdFromMuteControlSet(MAX_STREAMID - 1);
}

#ifdef HAS_FEATURE_INNERCAPTURER
/**
 * @tc.name  : Test ShouldBeInnerCap API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceShouldBeInnerCap_001
 * @tc.desc  : Test ShouldBeInnerCap interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceShouldBeInnerCap_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    bool ret = AudioService::GetInstance()->ShouldBeInnerCap(config, 0);
    EXPECT_FALSE(ret);
    config.privacyType = AudioPrivacyType::PRIVACY_TYPE_PRIVATE;
    ret = AudioService::GetInstance()->ShouldBeInnerCap(config, 0);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test ShouldBeDualTone API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceShouldBeDualTone_001
 * @tc.desc  : Test ShouldBeDualTone interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceShouldBeDualTone_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_RECORD;
    config.rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    bool ret = AudioService::GetInstance()->ShouldBeDualTone(config);
    EXPECT_FALSE(ret);
    config.audioMode = AUDIO_MODE_PLAYBACK;
    ret = AudioService::GetInstance()->ShouldBeDualTone(config);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test OnInitInnerCapList API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceOnInitInnerCapList_001
 * @tc.desc  : Test OnInitInnerCapList interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceOnInitInnerCapList_001, TestSize.Level1)
{
    int32_t floatRet = 0;

    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->InitAllDupBuffer(1);
    AudioService::GetInstance()->ResetAudioEndpoint();
    AudioService::GetInstance()->RenderersCheckForAudioWorkgroup(1);
    floatRet = AudioService::GetInstance()->GetMaxAmplitude(true);
    EXPECT_EQ(0, floatRet);

    AudioProcessConfig config = {};
    config.privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    AudioService::GetInstance()->GetAudioProcess(config);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->InitAllDupBuffer(1);
    AudioService::GetInstance()->RenderersCheckForAudioWorkgroup(1);
    AudioService::GetInstance()->workingConfig_.filterOptions.usages.emplace_back(STREAM_USAGE_MEDIA);
    AudioService::GetInstance()->OnInitInnerCapList(1);

    AudioService::GetInstance()->workingConfig_.filterOptions.pids.emplace_back(1);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->InitAllDupBuffer(1);
    AudioService::GetInstance()->RenderersCheckForAudioWorkgroup(1);
    AudioService::GetInstance()->OnUpdateInnerCapList(1);
    EXPECT_EQ(0, floatRet);
    config = {};
    config.privacyType = AudioPrivacyType::PRIVACY_TYPE_PRIVATE;
    config.audioMode = AUDIO_MODE_RECORD;
    config.streamType = STREAM_VOICE_CALL;
    config.streamInfo.channels = AudioChannel::MONO;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.encoding = ENCODING_PCM;
    auto audioProcess = AudioService::GetInstance()->GetAudioProcess(config);
    EXPECT_EQ(audioProcess, nullptr);

    AudioService::GetInstance()->OnInitInnerCapList(1);
    floatRet = AudioService::GetInstance()->GetMaxAmplitude(true);
    EXPECT_EQ(0, floatRet);
    floatRet = AudioService::GetInstance()->GetMaxAmplitude(false);
    EXPECT_EQ(0, floatRet);
    int32_t ret = AudioService::GetInstance()->EnableDualToneList(MAX_STREAMID - 1);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioService::GetInstance()->DisableDualToneList(MAX_STREAMID - 1);
    EXPECT_EQ(SUCCESS, ret);
    AudioService::GetInstance()->ResetAudioEndpoint();
    ret = AudioService::GetInstance()->OnProcessRelease(audioProcess, false);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test IsEndpointTypeVoip API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceIsEndpointTypeVoip_001
 * @tc.desc  : Test IsEndpointTypeVoip interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceIsEndpointTypeVoip_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor info(AudioDeviceDescriptor::DEVICE_INFO);
    config.rendererInfo.streamUsage = STREAM_USAGE_INVALID;
    config.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    config.rendererInfo.originalFlag = AUDIO_FLAG_VOIP_FAST;
    bool ret = AudioService::GetInstance()->IsEndpointTypeVoip(config, info);
    EXPECT_EQ(true, ret);

    config.capturerInfo.sourceType = SOURCE_TYPE_INVALID;
    ret = AudioService::GetInstance()->IsEndpointTypeVoip(config, info);
    EXPECT_FALSE(ret);

    config.rendererInfo.streamUsage = STREAM_USAGE_VIDEO_COMMUNICATION;
    ret = AudioService::GetInstance()->IsEndpointTypeVoip(config, info);
    EXPECT_TRUE(ret);

    config.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    ret = AudioService::GetInstance()->IsEndpointTypeVoip(config, info);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test GetCapturerBySessionID API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceGetCapturerBySessionID_001
 * @tc.desc  : Test GetCapturerBySessionID interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceGetCapturerBySessionID_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_RECORD;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = CH_LAYOUT_STEREO;
    config.streamType = STREAM_MUSIC;

    int32_t result;
    AudioService::GetInstance()->RemoveCapturer(-1);
    sptr<OHOS::AudioStandard::IpcStreamInServer> server = AudioService::GetInstance()->GetIpcStream(config, result);
    EXPECT_EQ(server, nullptr);

    auto ret = AudioService::GetInstance()->GetCapturerBySessionID(0);
    EXPECT_EQ(nullptr, ret);
}

/**
 * @tc.name  : Test ShouldBeDualTone API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceShouldBeDualTone_002
 * @tc.desc  : Test ShouldBeDualTone interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceShouldBeDualTone_002, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_RECORD;
    bool ret;
    ret = AudioService::GetInstance()->ShouldBeDualTone(config);
    EXPECT_EQ(ret, false);
    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_RINGTONE;
    ret = AudioService::GetInstance()->ShouldBeDualTone(config);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test FilterAllFastProcess API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceFilterAllFastProcess_001
 * @tc.desc  : Test FilterAllFastProcess interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceFilterAllFastProcess_001, TestSize.Level1)
{
    int32_t floatRet = 0;
    AudioService::GetInstance()->FilterAllFastProcess();
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->ResetAudioEndpoint();
    floatRet = AudioService::GetInstance()->GetMaxAmplitude(true);
    EXPECT_EQ(0, floatRet);

    AudioProcessConfig config = {};
    config.privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    AudioService::GetInstance()->GetAudioProcess(config);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->workingConfig_.filterOptions.usages.emplace_back(STREAM_USAGE_MEDIA);
    AudioService::GetInstance()->OnInitInnerCapList(1);

    AudioService::GetInstance()->workingConfig_.filterOptions.pids.emplace_back(1);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->OnUpdateInnerCapList(1);
    EXPECT_EQ(0, floatRet);
    AudioService::GetInstance()->FilterAllFastProcess();
}

/**
 * @tc.name  : Test GetDeviceInfoForProcess API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceGetDeviceInfoForProcess_001
 * @tc.desc  : Test GetDeviceInfoForProcess interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceGetDeviceInfoForProcess_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_PLAYBACK;
    AudioDeviceDescriptor deviceinfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamInfo info;
    deviceinfo = AudioService::GetInstance()->GetDeviceInfoForProcess(config, info);
    EXPECT_NE(deviceinfo.deviceRole_, INPUT_DEVICE);
    config.audioMode = AUDIO_MODE_RECORD;
    deviceinfo = AudioService::GetInstance()->GetDeviceInfoForProcess(config, info);
    EXPECT_NE(deviceinfo.deviceRole_, OUTPUT_DEVICE);
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceDump_001
 * @tc.desc  : Test Dump interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceDump_001, TestSize.Level1)
{
    int32_t floatRet = 0;
    AudioService::GetInstance()->FilterAllFastProcess();
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->InitAllDupBuffer(1);
    AudioService::GetInstance()->RenderersCheckForAudioWorkgroup(1);
    AudioService::GetInstance()->ResetAudioEndpoint();
    floatRet = AudioService::GetInstance()->GetMaxAmplitude(true);
    EXPECT_EQ(0, floatRet);

    AudioProcessConfig config = {};
    config.privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    AudioService::GetInstance()->GetAudioProcess(config);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->workingConfig_.filterOptions.usages.emplace_back(STREAM_USAGE_MEDIA);
    AudioService::GetInstance()->OnInitInnerCapList(1);

    AudioService::GetInstance()->workingConfig_.filterOptions.pids.emplace_back(1);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->InitAllDupBuffer(1);
    AudioService::GetInstance()->RenderersCheckForAudioWorkgroup(1);
    AudioService::GetInstance()->OnUpdateInnerCapList(1);
    EXPECT_EQ(0, floatRet);
    std::string dumpString = "This is Dump string";
    AudioService::GetInstance()->Dump(dumpString);
}

/**
 * @tc.name  : Test SetNonInterruptMute API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSetNonInterruptMute_001
 * @tc.desc  : Test SetNonInterruptMute interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceSetNonInterruptMute_001, TestSize.Level1)
{
    int32_t floatRet = 0;
    bool muteFlag = true;
    uint32_t sessionId = 0;

    AudioService::GetInstance()->FilterAllFastProcess();
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->ResetAudioEndpoint();
    AudioService::GetInstance()->SetNonInterruptMute(sessionId, muteFlag);
    floatRet = AudioService::GetInstance()->GetMaxAmplitude(true);
    EXPECT_EQ(0, floatRet);

    AudioProcessConfig config = {};
    config.privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    AudioService::GetInstance()->GetAudioProcess(config);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->workingConfig_.filterOptions.usages.emplace_back(STREAM_USAGE_MEDIA);
    AudioService::GetInstance()->OnInitInnerCapList(1);

    AudioService::GetInstance()->workingConfig_.filterOptions.pids.emplace_back(1);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->OnUpdateInnerCapList(1);
    AudioService::GetInstance()->SetNonInterruptMute(MAX_STREAMID - 1, muteFlag);
    EXPECT_EQ(0, floatRet);
}

/**
 * @tc.name  : Test OnProcessRelease API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceOnProcessRelease_001
 * @tc.desc  : Test OnProcessRelease interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceOnProcessRelease_001, TestSize.Level1)
{
    bool isSwitchStream = false;
    int32_t floatRet = 0;
    bool muteFlag = true;
    uint32_t sessionId = 0;

    AudioService::GetInstance()->FilterAllFastProcess();
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->ResetAudioEndpoint();
    AudioService::GetInstance()->SetNonInterruptMute(sessionId, muteFlag);
    floatRet = AudioService::GetInstance()->GetMaxAmplitude(true);
    EXPECT_EQ(0, floatRet);

    AudioProcessConfig config = {};
    config.privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, AudioService::GetInstance());
    EXPECT_NE(audioprocess, nullptr);
    audioprocess->Start();
    AudioService::GetInstance()->GetAudioProcess(config);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->workingConfig_.filterOptions.usages.emplace_back(STREAM_USAGE_MEDIA);
    AudioService::GetInstance()->OnInitInnerCapList(1);

    AudioService::GetInstance()->workingConfig_.filterOptions.pids.emplace_back(1);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->OnUpdateInnerCapList(1);

    int32_t ret = 0;
    ret = AudioService::GetInstance()->OnProcessRelease(audioprocess, isSwitchStream);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test OnProcessRelease API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceOnProcessRelease_002
 * @tc.desc  : Test OnProcessRelease interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceOnProcessRelease_002, TestSize.Level1)
{
    bool isSwitchStream = false;
    int32_t floatRet = 0;
    bool muteFlag = true;
    uint32_t sessionId = 0;

    AudioService::GetInstance()->FilterAllFastProcess();
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->ResetAudioEndpoint();
    AudioService::GetInstance()->SetNonInterruptMute(sessionId, muteFlag);
    floatRet = AudioService::GetInstance()->GetMaxAmplitude(true);
    EXPECT_EQ(0, floatRet);

    AudioProcessConfig config = {};
    config.privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    config.rendererInfo.isLoopback = true;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, AudioService::GetInstance());
    EXPECT_NE(audioprocess, nullptr);
    audioprocess->Start();
    AudioService::GetInstance()->GetAudioProcess(config);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->workingConfig_.filterOptions.usages.emplace_back(STREAM_USAGE_MEDIA);
    AudioService::GetInstance()->OnInitInnerCapList(1);

    AudioService::GetInstance()->workingConfig_.filterOptions.pids.emplace_back(1);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->OnUpdateInnerCapList(1);

    int32_t ret = 0;
    ret = AudioService::GetInstance()->OnProcessRelease(audioprocess, isSwitchStream);
    EXPECT_EQ(ret, 0);
}


/**
 * @tc.name  : Test OnProcessRelease API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceOnProcessRelease_003
 * @tc.desc  : Test OnProcessRelease interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceOnProcessRelease_003, TestSize.Level1)
{
    bool isSwitchStream = false;
    int32_t floatRet = 0;
    bool muteFlag = true;
    uint32_t sessionId = 0;

    AudioService::GetInstance()->FilterAllFastProcess();
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->ResetAudioEndpoint();
    AudioService::GetInstance()->SetNonInterruptMute(sessionId, muteFlag);
    floatRet = AudioService::GetInstance()->GetMaxAmplitude(true);
    EXPECT_EQ(0, floatRet);

    AudioProcessConfig config = {};
    config.privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    config.audioMode = AUDIO_MODE_RECORD;
    config.capturerInfo.isLoopback = true;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, AudioService::GetInstance());
    EXPECT_NE(audioprocess, nullptr);
    audioprocess->Start();
    AudioService::GetInstance()->GetAudioProcess(config);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->workingConfig_.filterOptions.usages.emplace_back(STREAM_USAGE_MEDIA);
    AudioService::GetInstance()->OnInitInnerCapList(1);

    AudioService::GetInstance()->workingConfig_.filterOptions.pids.emplace_back(1);
    AudioService::GetInstance()->OnInitInnerCapList(1);
    AudioService::GetInstance()->OnUpdateInnerCapList(1);

    int32_t ret = 0;
    ret = AudioService::GetInstance()->OnProcessRelease(audioprocess, isSwitchStream);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test GetAudioEndpointForDevice API
 * @tc.type  : FUNC
 * @tc.number: GetAudioEndpointForDevice_001
 * @tc.desc  : Test GetAudioEndpointForDevice interface.
 */
HWTEST(AudioServiceUnitTest, GetAudioEndpointForDevice_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    AudioProcessConfig clientConfig;
    clientConfig.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    clientConfig.rendererInfo.originalFlag = AUDIO_FLAG_VOIP_FAST;
    audioService->GetAudioProcess(clientConfig);
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: Dump_001
 * @tc.desc  : Test Dump interface.
 */
HWTEST(AudioServiceUnitTest, Dump_001, TestSize.Level1)
{
    std::string dumpString = "abcdefg";
    AudioService *audioService = AudioService::GetInstance();
    audioService->Dump(dumpString);

    AudioProcessConfig processConfig;

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();

    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;

    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);

    std::shared_ptr<RendererInServer> renderer = rendererInServer;

    audioService->InsertRenderer(1, renderer);
    audioService->workingConfigs_[1];
    audioService->Dump(dumpString);
    audioService->RemoveRenderer(1);
}

/**
 * @tc.name  : Test GetMaxAmplitude API
 * @tc.type  : FUNC
 * @tc.number: GetMaxAmplitude_001
 * @tc.desc  : Test GetMaxAmplitude interface.
 */
HWTEST(AudioServiceUnitTest, GetMaxAmplitude_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    int ret = audioService->GetMaxAmplitude(true);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name  : Test GetCapturerBySessionID API
 * @tc.type  : FUNC
 * @tc.number: GetCapturerBySessionID_001
 * @tc.desc  : Test GetCapturerBySessionID interface.
 */
HWTEST(AudioServiceUnitTest, GetCapturerBySessionID_001, TestSize.Level1)
{
    uint32_t sessionID = 2;
    AudioService *audioService = AudioService::GetInstance();
    std::shared_ptr<CapturerInServer> renderer = nullptr;
    audioService->InsertCapturer(1, renderer);
    std::shared_ptr<CapturerInServer> ret = audioService->GetCapturerBySessionID(sessionID);
    EXPECT_EQ(nullptr, ret);
    audioService->RemoveCapturer(1);
}

/**
 * @tc.name  : Test GetCapturerBySessionID API
 * @tc.type  : FUNC
 * @tc.number: GetCapturerBySessionID_002
 * @tc.desc  : Test GetCapturerBySessionID interface.
 */
HWTEST(AudioServiceUnitTest, GetCapturerBySessionID_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();

    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;

    std::shared_ptr<CapturerInServer> capturerInServer =
        std::make_shared<CapturerInServer>(processConfig, streamListener);

    std::shared_ptr<CapturerInServer> capturer = capturerInServer;
    uint32_t sessionID = 1;
    AudioService *audioService = AudioService::GetInstance();
    std::shared_ptr<CapturerInServer> renderer = nullptr;
    audioService->InsertCapturer(1, renderer);
    std::shared_ptr<CapturerInServer> ret = audioService->GetCapturerBySessionID(sessionID);
    EXPECT_EQ(nullptr, ret);
    audioService->RemoveCapturer(1);
}

/**
 * @tc.name  : Test SetNonInterruptMute API
 * @tc.type  : FUNC
 * @tc.number: SetNonInterruptMute_001
 * @tc.desc  : Test SetNonInterruptMute interface.
 */
HWTEST(AudioServiceUnitTest, SetNonInterruptMute_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    std::shared_ptr<RendererInServer> renderer = nullptr;
    audioService->InsertRenderer(1, renderer);
    audioService->SetNonInterruptMute(1, true);
    audioService->RemoveRenderer(1);
}

/**
 * @tc.name  : Test SetNonInterruptMute API
 * @tc.type  : FUNC
 * @tc.number: SetNonInterruptMute_002
 * @tc.desc  : Test SetNonInterruptMute interface.
 */
HWTEST(AudioServiceUnitTest, SetNonInterruptMute_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();

    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;

    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);

    std::shared_ptr<RendererInServer> renderer = rendererInServer;

    AudioService *audioService = AudioService::GetInstance();
    audioService->InsertRenderer(1, renderer);
    audioService->SetNonInterruptMute(1, true);
    audioService->RemoveRenderer(1);
}

/**
 * @tc.name  : Test SetNonInterruptMute API
 * @tc.type  : FUNC
 * @tc.number: SetNonInterruptMute_003
 * @tc.desc  : Test SetNonInterruptMute interface.
 */
HWTEST(AudioServiceUnitTest, SetNonInterruptMute_003, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    std::shared_ptr<CapturerInServer> capturer = nullptr;
    audioService->InsertCapturer(1, capturer);
    audioService->SetNonInterruptMute(1, true);
    audioService->RemoveCapturer(1);
}

/**
 * @tc.name  : Test SetNonInterruptMute API
 * @tc.type  : FUNC
 * @tc.number: SetNonInterruptMute_004
 * @tc.desc  : Test SetNonInterruptMute interface.
 */
HWTEST(AudioServiceUnitTest, SetNonInterruptMute_004, TestSize.Level1)
{
    AudioProcessConfig processConfig;

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();

    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;

    std::shared_ptr<CapturerInServer> capturerInServer =
        std::make_shared<CapturerInServer>(processConfig, streamListener);

    std::shared_ptr<CapturerInServer> capturer = capturerInServer;

    AudioService *audioService = AudioService::GetInstance();
    audioService->InsertCapturer(1, capturer);
    audioService->SetNonInterruptMute(1, true);
    audioService->RemoveCapturer(1);
}

/**
 * @tc.name  : Test SetNonInterruptMute API
 * @tc.type  : FUNC
 * @tc.number: SetNonInterruptMute_005
 * @tc.desc  : Test SetNonInterruptMute interface.
 */
HWTEST(AudioServiceUnitTest, SetNonInterruptMute_005, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->SetNonInterruptMute(1, true);
    EXPECT_EQ(1, audioService->muteSwitchStreams_.count(1));
    audioService->SetNonInterruptMute(1, false);
    EXPECT_EQ(0, audioService->muteSwitchStreams_.count(1));
    audioService->mutedSessions_.insert(1);
    audioService->SetNonInterruptMute(1, true);
    EXPECT_EQ(1, audioService->mutedSessions_.count(1));
    audioService->SetNonInterruptMute(1, false);
    EXPECT_EQ(0, audioService->mutedSessions_.count(1));
}

/**
 * @tc.name  : Test SetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadMode_001
 * @tc.desc  : Test SetOffloadMode interface.
 */
HWTEST(AudioServiceUnitTest, SetOffloadMode_001, TestSize.Level1)
{
    uint32_t sessionId = 2;
    int32_t state = 1;
    bool isAppBack = true;
    AudioService *audioService = AudioService::GetInstance();
    std::shared_ptr<CapturerInServer> capturer = nullptr;
    audioService->InsertCapturer(1, capturer);
    int32_t ret = audioService->SetOffloadMode(sessionId, state, isAppBack);
    EXPECT_EQ(ERR_INVALID_INDEX, ret);
    audioService->RemoveCapturer(1);
}

/**
 * @tc.name  : Test CheckRenderSessionMuteState API
 * @tc.type  : FUNC
 * @tc.number: CheckRenderSessionMuteState_001
 * @tc.desc  : Test CheckRenderSessionMuteState interface.
 */
HWTEST(AudioServiceUnitTest, CheckRenderSessionMuteState_001, TestSize.Level1)
{
    uint32_t sessionId = 2;
    AudioService *audioService = AudioService::GetInstance();
    audioService->UpdateMuteControlSet(sessionId, true);

    std::shared_ptr<RendererInServer> renderer = nullptr;
    audioService->CheckRenderSessionMuteState(sessionId, renderer);

    std::shared_ptr<CapturerInServer> capturer = nullptr;
    audioService->CheckCaptureSessionMuteState(sessionId, capturer);

    sptr<AudioProcessInServer> audioprocess = nullptr;
    audioService->CheckFastSessionMuteState(sessionId, audioprocess);

    audioService->RemoveIdFromMuteControlSet(sessionId);
    audioService->RemoveIdFromMuteControlSet(1);

    bool ret = audioService->IsExceedingMaxStreamCntPerUid(MEDIA_SERVICE_UID, 1, 0);
    EXPECT_EQ(ret, true);
    ret = audioService->IsExceedingMaxStreamCntPerUid(1, 1, 3);
    EXPECT_EQ(ret, false);
    int32_t mostAppUid = 1;
    int32_t mostAppNum = 1;
    audioService->GetCreatedAudioStreamMostUid(mostAppUid, mostAppNum);
}

/**
 * @tc.name  : Test CheckInnerCapForRenderer API
 * @tc.type  : FUNC
 * @tc.number: CheckInnerCapForRenderer_001
 * @tc.desc  : Test CheckInnerCapForRenderer interface.
 */
HWTEST(AudioServiceUnitTest, CheckInnerCapForRenderer_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    AudioProcessConfig processConfig;

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();

    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;

    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);

    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    audioService->CheckInnerCapForRenderer(1, renderer);
    audioService->workingConfigs_[1];
    audioService->CheckInnerCapForRenderer(1, renderer);
    int32_t ret = audioService->OnCapturerFilterRemove(1, 1);
    EXPECT_EQ(SUCCESS, ret);
    audioService->workingConfigs_.clear();
    ret = audioService->OnCapturerFilterRemove(1, 1);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test OnCapturerFilterChange API
 * @tc.type  : FUNC
 * @tc.number: OnCapturerFilterChange_001
 * @tc.desc  : Test OnCapturerFilterChange interface.
 */
HWTEST(AudioServiceUnitTest, OnCapturerFilterChange_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    AudioPlaybackCaptureConfig newConfig;
    int32_t ret = audioService->OnCapturerFilterChange(1, newConfig, 1);
    EXPECT_EQ(ret, 0);
    audioService->workingConfigs_[1];
    ret = audioService->OnCapturerFilterChange(1, newConfig, 1);
    EXPECT_EQ(ret, 0);
    audioService->workingConfigs_.clear();
}

/**
 * @tc.name  : Test ShouldBeInnerCap API
 * @tc.type  : FUNC
 * @tc.number: ShouldBeInnerCap_001
 * @tc.desc  : Test ShouldBeInnerCap interface.
 */
HWTEST(AudioServiceUnitTest, ShouldBeInnerCap_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    AudioService *audioService = AudioService::GetInstance();
    int32_t ret = audioService->ShouldBeInnerCap(config, 0);
    EXPECT_FALSE(ret);
    audioService->workingConfig_.filterOptions.usages.push_back(STREAM_USAGE_MUSIC);
    ret = audioService->ShouldBeInnerCap(config, 0);
    EXPECT_FALSE(ret);
    audioService->workingConfig_.filterOptions.pids.push_back(1);
    ret = audioService->ShouldBeInnerCap(config, 0);
    EXPECT_FALSE(ret);
}
#endif


/**
 * @tc.name  : Test CheckRenderSessionMuteState API
 * @tc.type  : FUNC
 * @tc.number: CheckRenderSessionMuteState_002
 * @tc.desc  : Test CheckRenderSessionMuteState interface.
 */
HWTEST(AudioServiceUnitTest, CheckRenderSessionMuteState_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = 100001;
    AudioService *audioService = AudioService::GetInstance();
    audioService->UpdateMuteControlSet(sessionId, true);

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();

    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    audioService->CheckRenderSessionMuteState(sessionId, renderer);

    audioService->RemoveIdFromMuteControlSet(sessionId);

    bool ret = audioService->IsExceedingMaxStreamCntPerUid(MEDIA_SERVICE_UID, 1, 0);
    EXPECT_EQ(ret, true);
    ret = audioService->IsExceedingMaxStreamCntPerUid(1, 1, 3);
    EXPECT_EQ(ret, false);
    int32_t mostAppUid = 1;
    int32_t mostAppNum = 1;
    audioService->GetCreatedAudioStreamMostUid(mostAppUid, mostAppNum);
}
/**
 * @tc.name  : Test CheckRenderSessionMuteState API
 * @tc.type  : FUNC
 * @tc.number: heckRenderSessionMuteState_003
 * @tc.desc  : Test CheckRenderSessionMuteState interface.
 */
HWTEST(AudioServiceUnitTest, CheckRenderSessionMuteState_003, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = 100001;
    AudioService *audioService = AudioService::GetInstance();
    audioService->UpdateMuteControlSet(sessionId, true);

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();

    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;

    std::shared_ptr<CapturerInServer> capturerInServer =
        std::make_shared<CapturerInServer>(processConfig, streamListener);
    std::shared_ptr<CapturerInServer> capturer = capturerInServer;
    audioService->CheckCaptureSessionMuteState(sessionId, capturer);

    audioService->RemoveIdFromMuteControlSet(sessionId);

    bool ret = audioService->IsExceedingMaxStreamCntPerUid(MEDIA_SERVICE_UID, 1, 0);
    EXPECT_EQ(ret, true);
    ret = audioService->IsExceedingMaxStreamCntPerUid(1, 1, 3);
    EXPECT_EQ(ret, false);
    int32_t mostAppUid = 1;
    int32_t mostAppNum = 1;
    audioService->GetCreatedAudioStreamMostUid(mostAppUid, mostAppNum);
}
/**
 * @tc.name  : Test CheckRenderSessionMuteState API
 * @tc.type  : FUNC
 * @tc.number: CheckRenderSessionMuteState_004
 * @tc.desc  : Test CheckRenderSessionMuteState interface.
 */
HWTEST(AudioServiceUnitTest, CheckRenderSessionMuteState_004, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = 100001;
    AudioService *audioService = AudioService::GetInstance();
    audioService->UpdateMuteControlSet(sessionId, true);

    sptr<AudioProcessInServer> audioprocess = AudioProcessInServer::Create(processConfig, AudioService::GetInstance());;
    audioService->CheckFastSessionMuteState(sessionId, audioprocess);

    audioService->RemoveIdFromMuteControlSet(sessionId);

    bool ret = audioService->IsExceedingMaxStreamCntPerUid(MEDIA_SERVICE_UID, 1, 0);
    EXPECT_EQ(ret, true);
    ret = audioService->IsExceedingMaxStreamCntPerUid(1, 1, 3);
    EXPECT_EQ(ret, true);
    int32_t mostAppUid = 1;
    int32_t mostAppNum = 1;
    audioService->GetCreatedAudioStreamMostUid(mostAppUid, mostAppNum);
}

/**
 * @tc.name  : Test CheckRenderSessionMuteState API
 * @tc.type  : FUNC
 * @tc.number: CheckRenderSessionMuteState_005
 * @tc.desc  : Test CheckRenderSessionMuteState interface.
 */
HWTEST(AudioServiceUnitTest, CheckRenderSessionMuteState_005, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = 100001;
    AudioService *audioService = AudioService::GetInstance();

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();

    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    audioService->muteSwitchStreams_.insert(sessionId);
    audioService->CheckRenderSessionMuteState(sessionId, renderer);
    EXPECT_EQ(audioService->mutedSessions_.count(sessionId), 0);
    audioService->RemoveIdFromMuteControlSet(sessionId);
}

/**
 * @tc.name  : Test CheckCapturerSessionMuteState API
 * @tc.type  : FUNC
 * @tc.number: CheckCapturerSessionMuteState_006
 * @tc.desc  : Test CheckCapturerSessionMuteState interface.
 */
HWTEST(AudioServiceUnitTest, CheckCapturerSessionMuteState_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = 100001;
    AudioService *audioService = AudioService::GetInstance();

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();

    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<CapturerInServer> capturerInServer =
        std::make_shared<CapturerInServer>(processConfig, streamListener);
    std::shared_ptr<CapturerInServer> capturer = capturerInServer;
    audioService->muteSwitchStreams_.insert(sessionId);
    audioService->CheckCaptureSessionMuteState(sessionId, capturer);
    EXPECT_EQ(audioService->mutedSessions_.count(sessionId), 0);
    audioService->RemoveIdFromMuteControlSet(sessionId);
}

/**
 * @tc.name  : Test CheckFastSessionMuteState API
 * @tc.type  : FUNC
 * @tc.number: CheckFastSessionMuteState_006
 * @tc.desc  : Test CheckFastSessionMuteState interface.
 */
HWTEST(AudioServiceUnitTest, CheckFastSessionMuteState_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = 100001;
    AudioService *audioService = AudioService::GetInstance();

    sptr<AudioProcessInServer> audioprocess = AudioProcessInServer::Create(processConfig, AudioService::GetInstance());;
    audioService->CheckFastSessionMuteState(sessionId, audioprocess);

    audioService->muteSwitchStreams_.insert(sessionId);
    audioService->CheckFastSessionMuteState(sessionId, audioprocess);
    EXPECT_EQ(audioService->mutedSessions_.count(sessionId), 0);
    audioService->RemoveIdFromMuteControlSet(sessionId);
}

/**
 * @tc.name  : Test GetStandbyStatus API
 * @tc.type  : FUNC
 * @tc.number: GetStandbyStatus_001
 * @tc.desc  : Test GetStandbyStatus interface.
 */
HWTEST(AudioServiceUnitTest, GetStandbyStatus_001, TestSize.Level1)
{
    uint32_t sessionId = 100001;
    AudioService *audioService = AudioService::GetInstance();
    audioService->UpdateMuteControlSet(sessionId, true);
    bool isStandby = false;
    int64_t enterStandbyTime = 100000;
    int ret = audioService->GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}
/**
 * @tc.name  : Test OnUpdateInnerCapList API
 * @tc.type  : FUNC
 * @tc.number: OnUpdateInnerCapList_001
 * @tc.desc  : Test OnUpdateInnerCapList interface.
 */
HWTEST(AudioServiceUnitTest, OnUpdateInnerCapList_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    std::shared_ptr<RendererInServer> renderer = nullptr;
    std::vector<std::weak_ptr<RendererInServer>> rendererVector;
    rendererVector.push_back(renderer);
    int32_t innerCapId = 1;
    audioService->filteredRendererMap_.insert(std::make_pair(innerCapId, rendererVector));
    int32_t ret = audioService->OnUpdateInnerCapList(innerCapId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test EnableDualToneList API
 * @tc.type  : FUNC
 * @tc.number: EnableDualToneList_001
 * @tc.desc  : Test EnableDualToneList interface.
 */
HWTEST(AudioServiceUnitTest, EnableDualToneList_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    std::shared_ptr<RendererInServer> renderer = nullptr;
    int32_t sessionId = 1;
    audioService->allRendererMap_.insert(std::make_pair(sessionId, renderer));
    int32_t ret = audioService->EnableDualToneList(sessionId);
    EXPECT_EQ(ret, SUCCESS);
}
/**
 * @tc.name  : Test DisableDualToneList API
 * @tc.type  : FUNC
 * @tc.number: DisableDualToneList_001
 * @tc.desc  : Test DisableDualToneList interface.
 */
HWTEST(AudioServiceUnitTest, DisableDualToneList_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    std::shared_ptr<RendererInServer> renderer = nullptr;
    audioService->filteredDualToneRendererMap_.push_back(renderer);
    int32_t sessionId = 1;
    int32_t ret = audioService->DisableDualToneList(sessionId);
    EXPECT_EQ(ret, SUCCESS);
}
/**
 * @tc.name  : Test UpdateAudioSinkState API
 * @tc.type  : FUNC
 * @tc.number: UpdateAudioSinkState_001
 * @tc.desc  : Test UpdateAudioSinkState interface.
 */
HWTEST(AudioServiceUnitTest, UpdateAudioSinkState_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->currentRendererStreamCnt_ = 0;
    audioService->UpdateAudioSinkState(1, false);

    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    audioService->SetIncMaxRendererStreamCnt(audioMode);
    int32_t res = audioService->GetCurrentRendererStreamCnt();
    EXPECT_EQ(res, 1);
}
/**
 * @tc.name  : Test UpdateAudioSinkState API
 * @tc.type  : FUNC
 * @tc.number: UpdateAudioSinkState_002
 * @tc.desc  : Test UpdateAudioSinkState interface.
 */
HWTEST(AudioServiceUnitTest, UpdateAudioSinkState_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->currentRendererStreamCnt_ = 0;
    audioService->UpdateAudioSinkState(1, true);

    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    audioService->SetIncMaxRendererStreamCnt(audioMode);
    int32_t res = audioService->GetCurrentRendererStreamCnt();
    EXPECT_EQ(res, 1);
}
/**
 * @tc.name  : Test ShouldBeDualTone API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceShouldBeDualTone_003
 * @tc.desc  : Test ShouldBeDualTone interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceShouldBeDualTone_003, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_PLAYBACK;
    bool ret;
    ret = AudioService::GetInstance()->ShouldBeDualTone(config);
    EXPECT_EQ(ret, false);
    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_RINGTONE;
    ret = AudioService::GetInstance()->ShouldBeDualTone(config);
    EXPECT_FALSE(ret);
}
/**
 * @tc.name  : Test CheckHibernateState API
 * @tc.type  : FUNC
 * @tc.number: CheckHibernateState_001
 * @tc.desc  : Test CheckHibernateState interface.
 */
HWTEST(AudioServiceUnitTest, CheckHibernateState_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->currentRendererStreamCnt_ = 0;
    audioService->CheckHibernateState(true);

    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    audioService->SetIncMaxRendererStreamCnt(audioMode);
    int32_t res = audioService->GetCurrentRendererStreamCnt();
    EXPECT_EQ(res, 1);
}
/**
 * @tc.name  : Test CheckHibernateState API
 * @tc.type  : FUNC
 * @tc.number: CheckHibernateState_002
 * @tc.desc  : Test CheckHibernateState interface.
 */
HWTEST(AudioServiceUnitTest, CheckHibernateState_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->currentRendererStreamCnt_ = 0;
    audioService->CheckHibernateState(false);

    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    audioService->SetIncMaxRendererStreamCnt(audioMode);
    int32_t res = audioService->GetCurrentRendererStreamCnt();
    EXPECT_EQ(res, 1);
}
/**
 * @tc.name  : Test CheckHibernateState API
 * @tc.type  : FUNC
 * @tc.number: CheckHibernateState_003
 * @tc.desc  : Test CheckHibernateState interface.
 */
HWTEST(AudioServiceUnitTest, CheckHibernateState_003, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->allRunningSinks_.insert(1);
    audioService->currentRendererStreamCnt_ = 0;
    audioService->CheckHibernateState(true);

    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    audioService->SetIncMaxRendererStreamCnt(audioMode);
    int32_t res = audioService->GetCurrentRendererStreamCnt();
    EXPECT_EQ(res, 1);
}
/**
 * @tc.name  : Test UnsetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: UnsetOffloadMode_001
 * @tc.desc  : Test UnsetOffloadMode interface.
 */
HWTEST(AudioServiceUnitTest, UnsetOffloadMode_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->allRunningSinks_.insert(1);
    int ret = audioService->UnsetOffloadMode(1);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test DelayCallReleaseEndpoint API
 * @tc.type  : FUNC
 * @tc.number: DelayCallReleaseEndpoint_001
 * @tc.desc  : Test DelayCallReleaseEndpoint interface.
 */
HWTEST(AudioServiceUnitTest, DelayCallReleaseEndpoint_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_MMAP,
        123, clientConfig);
    EXPECT_NE(nullptr, endpoint);
    string endpointName = endpoint->GetEndpointName();
    audioService->endpointList_[endpointName] = endpoint;

    audioService->releasingEndpointSet_.insert(endpointName);
    audioService->DelayCallReleaseEndpoint(endpointName);
    EXPECT_EQ(audioService->endpointList_.count(endpointName), 1);

    audioService->releasingEndpointSet_.insert(endpointName);
    endpoint->endpointStatus_ = AudioEndpoint::EndpointStatus::UNLINKED;
    audioService->DelayCallReleaseEndpoint(endpointName);
    EXPECT_EQ(audioService->endpointList_.count(endpointName), 0);
}

/**
 * @tc.name  : Test ReleaseProcess API
 * @tc.type  : FUNC
 * @tc.number: ReleaseProcess_001
 * @tc.desc  : Test ReleaseProcess interface.
 */
HWTEST(AudioServiceUnitTest, ReleaseProcess_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    std::string endpointName = "invalid_endpoint";
    audioService->ReleaseProcess(endpointName, 0);
}

/**
 * @tc.name  : Test ReleaseProcess API
 * @tc.type  : FUNC
 * @tc.number: ReleaseProcess_002
 * @tc.desc  : Test ReleaseProcess interface.
 */
HWTEST(AudioServiceUnitTest, ReleaseProcess_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    std::string endpointName = "endpoint";
    std::shared_ptr<AudioEndpoint> audioEndpoint = nullptr;
    int32_t delayInMs = 1;
    audioService->endpointList_[endpointName] = audioEndpoint;
    audioService->ReleaseProcess(endpointName, delayInMs);
    EXPECT_EQ(audioService->endpointList_.count(endpointName), 1);
    audioService->endpointList_.erase(endpointName);

    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    audioService->SetIncMaxRendererStreamCnt(audioMode);

    audioService->currentRendererStreamCnt_ = 0;
    int32_t res = audioService->GetCurrentRendererStreamCnt();
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name  : Test ReleaseProcess API
 * @tc.type  : FUNC
 * @tc.number: ReleaseProcess_003
 * @tc.desc  : Test ReleaseProcess interface.
 */
HWTEST(AudioServiceUnitTest, ReleaseProcess_003, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->currentRendererStreamCnt_ = 0;
    audioService->ReleaseProcess("endponit", 0);

    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    audioService->SetIncMaxRendererStreamCnt(audioMode);
    int32_t res = audioService->GetCurrentRendererStreamCnt();
    EXPECT_EQ(res, 1);
}

/**
 * @tc.name  : Test ReleaseProcess API
 * @tc.type  : FUNC
 * @tc.number: ReleaseProcess_004
 * @tc.desc  : Test ReleaseProcess interface.
 */
HWTEST(AudioServiceUnitTest, ReleaseProcess_004, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->currentRendererStreamCnt_ = 0;
    audioService->releasingEndpointSet_.insert("endponit");
    audioService->ReleaseProcess("endponit", 1);

    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    audioService->SetIncMaxRendererStreamCnt(audioMode);
    int32_t res = audioService->GetCurrentRendererStreamCnt();
    EXPECT_EQ(res, 1);
}

/**
 * @tc.name  : Test GetReleaseDelayTime API
 * @tc.type  : FUNC
 * @tc.number: GetReleaseDelayTime_001
 * @tc.desc  : Test GetReleaseDelayTime interface.
 */
HWTEST(AudioServiceUnitTest, GetReleaseDelayTime_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpoint> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        123, clientConfig);
    EXPECT_NE(nullptr, endpoint);

    bool isSwitchStream = false;
    int ret = audioService->GetReleaseDelayTime(endpoint, isSwitchStream, false);
    EXPECT_EQ(ret, VOIP_ENDPOINT_RELEASE_DELAY_TIME);
    ret = audioService->GetReleaseDelayTime(endpoint, isSwitchStream, true);
    EXPECT_EQ(ret, VOIP_REC_ENDPOINT_RELEASE_DELAY_TIME);
}

/**
 * @tc.name  : Test GetReleaseDelayTime API
 * @tc.type  : FUNC
 * @tc.number: GetReleaseDelayTime_002
 * @tc.desc  : Test GetReleaseDelayTime interface.
 */
HWTEST(AudioServiceUnitTest, GetReleaseDelayTime_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpoint> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_MMAP,
        123, clientConfig);
    EXPECT_NE(nullptr, endpoint);

    bool isSwitchStream = false;
    int ret = audioService->GetReleaseDelayTime(endpoint, isSwitchStream, false);
    EXPECT_EQ(ret, NORMAL_ENDPOINT_RELEASE_DELAY_TIME_MS);
    ret = audioService->GetReleaseDelayTime(endpoint, isSwitchStream, true);
    EXPECT_EQ(ret, NORMAL_ENDPOINT_RELEASE_DELAY_TIME_MS);
}

/**
 * @tc.name  : Test GetReleaseDelayTime API
 * @tc.type  : FUNC
 * @tc.number: GetReleaseDelayTime_003
 * @tc.desc  : Test GetReleaseDelayTime interface.
 */
HWTEST(AudioServiceUnitTest, GetReleaseDelayTime_003, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_MMAP,
        123, clientConfig);
    EXPECT_NE(nullptr, endpoint);

    endpoint->deviceInfo_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    bool isSwitchStream = false;
    int ret = audioService->GetReleaseDelayTime(endpoint, isSwitchStream, false);
    EXPECT_EQ(ret, A2DP_ENDPOINT_RELEASE_DELAY_TIME);
    ret = audioService->GetReleaseDelayTime(endpoint, isSwitchStream, true);
    EXPECT_EQ(ret, A2DP_ENDPOINT_RELEASE_DELAY_TIME);
    isSwitchStream = true;
    ret = audioService->GetReleaseDelayTime(endpoint, isSwitchStream, false);
    EXPECT_EQ(ret, A2DP_ENDPOINT_RE_CREATE_RELEASE_DELAY_TIME);
    ret = audioService->GetReleaseDelayTime(endpoint, isSwitchStream, true);
    EXPECT_EQ(ret, A2DP_ENDPOINT_RE_CREATE_RELEASE_DELAY_TIME);
}

/**
 * @tc.name  : Test GetStandbyStatus API
 * @tc.type  : FUNC
 * @tc.number: GetStandbyStatus_002
 * @tc.desc  : Test GetStandbyStatus interface.
 */
HWTEST(AudioServiceUnitTest, GetStandbyStatus_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    AudioProcessConfig processConfig;

    std::shared_ptr<RendererInServer> rendererInServer1 = nullptr;

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    EXPECT_NE(streamListenerHolder, nullptr);
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer2 =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(rendererInServer2, nullptr);

    audioService->allRendererMap_.clear();
    audioService->allRendererMap_.insert(std::make_pair(0, rendererInServer1));
    audioService->allRendererMap_.insert(std::make_pair(1, rendererInServer2));

    uint32_t sessionId = 0;
    bool isStandby = true;
    int64_t enterStandbyTime = 0;
    int ret = audioService->GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    sessionId = 1;
    ret = audioService->GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
    EXPECT_EQ(ret, SUCCESS);

    audioService->allRendererMap_.clear();
}

/**
 * @tc.name  : Test RemoveRenderer API
 * @tc.type  : FUNC
 * @tc.number: RemoveRenderer_001
 * @tc.desc  : Test RemoveRenderer interface.
 */
HWTEST(AudioServiceUnitTest, RemoveRenderer_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    audioService->allRendererMap_.clear();

    uint32_t sessionId = 100001;
    audioService->UpdateMuteControlSet(sessionId, true);

    std::set<uint32_t>::iterator end = audioService->mutedSessions_.end();
    std::set<uint32_t>::iterator it = end;
    audioService->RemoveRenderer(sessionId, true);
    {
        std::lock_guard<std::mutex> lock(audioService->mutedSessionsMutex_);
        it = audioService->mutedSessions_.find(sessionId);
        EXPECT_NE(it, end);
    }

    audioService->RemoveRenderer(sessionId, false);
    {
        std::lock_guard<std::mutex> lock(audioService->mutedSessionsMutex_);
        it = audioService->mutedSessions_.find(sessionId);
        EXPECT_NE(it, end);
    }
}

/**
 * @tc.name  : Test AddFilteredRender API
 * @tc.type  : FUNC
 * @tc.number: AddFilteredRender_001
 * @tc.desc  : Test AddFilteredRender interface.
 */
HWTEST(AudioServiceUnitTest, AddFilteredRender_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    audioService->filteredRendererMap_.clear();

    int32_t innerCapId = 0;
    std::shared_ptr<RendererInServer> renderer = nullptr;
    audioService->AddFilteredRender(innerCapId, renderer);
}

/**
 * @tc.name  : Test CheckInnerCapForRenderer API
 * @tc.type  : FUNC
 * @tc.number: CheckInnerCapForRenderer_002
 * @tc.desc  : Test CheckInnerCapForRenderer interface.
 */
HWTEST(AudioServiceUnitTest, CheckInnerCapForRenderer_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    audioService->workingConfigs_.clear();

    uint32_t sessionId = 0;
    std::shared_ptr<RendererInServer> renderer = nullptr;
    audioService->CheckInnerCapForRenderer(sessionId, renderer);
}

/**
 * @tc.name  : Test ShouldBeInnerCap API
 * @tc.type  : FUNC
 * @tc.number: ShouldBeInnerCap_002
 * @tc.desc  : Test ShouldBeInnerCap interface.
 */
HWTEST(AudioServiceUnitTest, ShouldBeInnerCap_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    audioService->workingConfigs_.clear();

    AudioProcessConfig rendererConfig;
    rendererConfig.privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    int32_t innerCapId = 1;
    bool ret = audioService->ShouldBeInnerCap(rendererConfig, innerCapId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test ShouldBeInnerCap API
 * @tc.type  : FUNC
 * @tc.number: ShouldBeInnerCap_003
 * @tc.desc  : Test ShouldBeInnerCap interface.
 */
HWTEST(AudioServiceUnitTest, ShouldBeInnerCap_003, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    AudioProcessConfig rendererConfig;
    rendererConfig.privacyType = AudioPrivacyType::PRIVACY_TYPE_PRIVATE;
    std::set<int32_t> beCapIds;
    bool ret = audioService->ShouldBeInnerCap(rendererConfig, beCapIds);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test CheckShouldCap API
 * @tc.type  : FUNC
 * @tc.number: CheckShouldCap_001
 * @tc.desc  : Test CheckShouldCap interface.
 */
HWTEST(AudioServiceUnitTest, CheckShouldCap_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    audioService->filteredRendererMap_.clear();

    AudioPlaybackCaptureConfig audioPlaybackCaptureConfig;
    audioPlaybackCaptureConfig.filterOptions.usages.push_back(STREAM_USAGE_MEDIA);
    audioService->workingConfigs_.clear();
    audioService->workingConfigs_.insert(std::make_pair(1, audioPlaybackCaptureConfig));

    int32_t innerCapId = 0;
    AudioProcessConfig rendererConfig;
    bool ret = audioService->CheckShouldCap(rendererConfig, innerCapId);
    EXPECT_EQ(ret, false);
    innerCapId = 1;
    ret = audioService->CheckShouldCap(rendererConfig, innerCapId);
    EXPECT_EQ(ret, false);
    audioPlaybackCaptureConfig.filterOptions.pids.push_back(1);
    ret = audioService->CheckShouldCap(rendererConfig, innerCapId);
    EXPECT_EQ(ret, false);

    audioService->workingConfigs_.clear();
}

/**
 * @tc.name  : Test FilterAllFastProcess API
 * @tc.type  : FUNC
 * @tc.number: FilterAllFastProcess_002
 * @tc.desc  : Test FilterAllFastProcess interface.
 */
HWTEST(AudioServiceUnitTest, FilterAllFastProcess_002, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_PLAYBACK;
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, AudioService::GetInstance());
    EXPECT_NE(audioprocess, nullptr);

    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        123, clientConfig);
    EXPECT_NE(endpoint, nullptr);
    endpoint->deviceInfo_.deviceRole_ = OUTPUT_DEVICE;

    audioService->linkedPairedList_.clear();
    audioService->linkedPairedList_.push_back(std::make_pair(audioprocess, endpoint));

    audioService->endpointList_.clear();
    audioService->endpointList_.insert(std::make_pair("endpoint", endpoint));

    audioService->FilterAllFastProcess();

    audioService->linkedPairedList_.clear();
    audioService->endpointList_.clear();
}

/**
 * @tc.name  : Test CheckDisableFastInner API
 * @tc.type  : FUNC
 * @tc.number: CheckDisableFastInner_001
 * @tc.desc  : Test CheckDisableFastInner interface.
 */
HWTEST(AudioServiceUnitTest, CheckDisableFastInner_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    AudioPlaybackCaptureConfig audioPlaybackCaptureConfig;
    audioService->workingConfigs_.clear();
    audioService->workingConfigs_.insert(std::make_pair(1, audioPlaybackCaptureConfig));

    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        123, clientConfig);
    int32_t ret = audioService->CheckDisableFastInner(endpoint);
    EXPECT_EQ(ret, SUCCESS);

    audioService->workingConfigs_.clear();
}

/**
 * @tc.name  : Test HandleFastCapture API
 * @tc.type  : FUNC
 * @tc.number: HandleFastCapture_001
 * @tc.desc  : Test HandleFastCapture interface.
 */
HWTEST(AudioServiceUnitTest, HandleFastCapture_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    audioService->filteredRendererMap_.clear();

    std::set<int32_t> captureIds = {1};
    AudioProcessConfig config = {};
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, AudioService::GetInstance());
    EXPECT_NE(audioprocess, nullptr);

    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        123, clientConfig);
    EXPECT_NE(endpoint, nullptr);

    int32_t ret = audioService->HandleFastCapture(captureIds, audioprocess, endpoint);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test OnInitInnerCapList API
 * @tc.type  : FUNC
 * @tc.number: OnInitInnerCapList_001
 * @tc.desc  : Test OnInitInnerCapList interface.
 */
HWTEST(AudioServiceUnitTest, OnInitInnerCapList_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);

    std::shared_ptr<RendererInServer> rendererInServer1 = nullptr;

    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    EXPECT_NE(streamListenerHolder, nullptr);
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer2 =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(rendererInServer2, nullptr);

    audioService->allRendererMap_.clear();
    audioService->allRendererMap_.insert(std::make_pair(0, rendererInServer1));
    audioService->allRendererMap_.insert(std::make_pair(1, rendererInServer2));

    int32_t innerCapId = 0;
    int32_t ret = audioService->OnInitInnerCapList(innerCapId);
    AudioService::GetInstance()->InitAllDupBuffer(1);
    AudioService::GetInstance()->RenderersCheckForAudioWorkgroup(1);
    EXPECT_EQ(ret, SUCCESS);

    audioService->allRendererMap_.clear();
}

/**
 * @tc.name  : Test SetDefaultAdapterEnable API
 * @tc.type  : FUNC
 * @tc.number: SetDefaultAdapterEnable_001
 * @tc.desc  : Test SetDefaultAdapterEnable interface.
 */
HWTEST(AudioServiceUnitTest, SetDefaultAdapterEnable_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    EXPECT_NE(audioService, nullptr);
    bool isEnable = false;
    audioService->SetDefaultAdapterEnable(isEnable);
    bool result = audioService->GetDefaultAdapterEnable();
    EXPECT_EQ(result, isEnable);
}

/*
 * @tc.name  : Test SetSessionMuteState API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSetSessionMuteState_001
 * @tc.desc  : Test RegisterMuteStateChangeCallback whether callback can invoke.
 */
HWTEST(AudioServiceUnitTest, AudioServiceSetSessionMuteState_001, TestSize.Level1)
{
    bool muteFlag = true;
    uint32_t sessionId = 0;
    uint32_t sessionId2 = 1;
    uint32_t sessionId3 = 3;
    bool testFlag = false;

    auto service = AudioService::GetInstance();
    ASSERT_NE(service, nullptr);
    service->SetSessionMuteState(sessionId, true, muteFlag);
    service->RegisterMuteStateChangeCallback(sessionId2, [&](bool flag) {
        testFlag = flag;
    });
    EXPECT_EQ(testFlag, false);
    service->RegisterMuteStateChangeCallback(sessionId, [&](bool flag) {
        testFlag = flag;
    });
    EXPECT_EQ(testFlag, false);

    testFlag = false;
    service->SetLatestMuteState(sessionId3, muteFlag);
    EXPECT_FALSE(testFlag);

    service->RegisterMuteStateChangeCallback(sessionId3, [&](bool flag) {
        testFlag = flag;
    });
    service->SetSessionMuteState(sessionId3, true, muteFlag);
    service->SetLatestMuteState(sessionId3, muteFlag);
    EXPECT_EQ(testFlag, muteFlag);
}

/**
 * @tc.name  : Test SetSessionMuteState API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceSetSessionMuteState_002
 * @tc.desc  : Test SetSessionMuteState interface .
 */
HWTEST(AudioServiceUnitTest, AudioServiceSetSessionMuteState_002, TestSize.Level1)
{
    bool muteFlag = true;
    uint32_t sessionId = 0;
    auto service = AudioService::GetInstance();
    ASSERT_NE(service, nullptr);
    service->SetSessionMuteState(sessionId, true, muteFlag);
    EXPECT_TRUE(service->muteStateMap_.count(sessionId) != 0);
    service->SetSessionMuteState(sessionId, false, muteFlag);
    EXPECT_TRUE(service->muteStateMap_.count(sessionId) == 0);
}

/**
 * @tc.name  : Test GetCurrentLoopbackStreamCnt API
 * @tc.type  : FUNC
 * @tc.number: AudioServiceLoopbackStreamCnt_001,
 * @tc.desc  : Test GetCurrentLoopbackStreamCnt interface.
 */
HWTEST(AudioServiceUnitTest, AudioServiceLoopbackStreamCnt_001, TestSize.Level1)
{
    int32_t rendererCnt = AudioService::GetInstance()->GetCurrentLoopbackStreamCnt(AUDIO_MODE_PLAYBACK);
    int32_t capturerCnt = AudioService::GetInstance()->GetCurrentLoopbackStreamCnt(AUDIO_MODE_RECORD);
    EXPECT_EQ(rendererCnt, 0);
    EXPECT_EQ(capturerCnt, 0);
    AudioService::GetInstance()->SetIncMaxLoopbackStreamCnt(AUDIO_MODE_PLAYBACK);
    AudioService::GetInstance()->SetIncMaxLoopbackStreamCnt(AUDIO_MODE_RECORD);
    rendererCnt = AudioService::GetInstance()->GetCurrentLoopbackStreamCnt(AUDIO_MODE_PLAYBACK);
    capturerCnt = AudioService::GetInstance()->GetCurrentLoopbackStreamCnt(AUDIO_MODE_RECORD);
    EXPECT_EQ(rendererCnt, 1);
    EXPECT_EQ(capturerCnt, 1);
    AudioService::GetInstance()->SetDecMaxLoopbackStreamCnt(AUDIO_MODE_PLAYBACK);
    AudioService::GetInstance()->SetDecMaxLoopbackStreamCnt(AUDIO_MODE_RECORD);
    rendererCnt = AudioService::GetInstance()->GetCurrentLoopbackStreamCnt(AUDIO_MODE_PLAYBACK);
    capturerCnt = AudioService::GetInstance()->GetCurrentLoopbackStreamCnt(AUDIO_MODE_RECORD);
    EXPECT_EQ(rendererCnt, 0);
    EXPECT_EQ(capturerCnt, 0);
}

/**
 * @tc.name  : Test SaveForegroundList API
 * @tc.type  : FUNC
 * @tc.number: SaveForegroundList_001,
 * @tc.desc  : Test SaveForegroundList interface.
 */
HWTEST(AudioServiceUnitTest, SaveForegroundList_001, TestSize.Level1)
{
    std::vector<std::string> list;
    list.resize(11);
    EXPECT_EQ(list.size(), 11);
    AudioService::GetInstance()->SaveForegroundList(list);
    EXPECT_EQ(AudioService::GetInstance()->foregroundSet_.size(), 0);
    EXPECT_EQ(AudioService::GetInstance()->foregroundUidSet_.size(), 0);

    list.resize(5);
    EXPECT_EQ(list.size(), 5);
    AudioService::GetInstance()->SaveForegroundList(list);
}

/**
 * @tc.name  : Test MatchForegroundList API
 * @tc.type  : FUNC
 * @tc.number: MatchForegroundList_001,
 * @tc.desc  : Test MatchForegroundList interface.
 */
HWTEST(AudioServiceUnitTest, MatchForegroundList_001, TestSize.Level1)
{
    uint32_t uid = 0;
    std::string bundleName = "test";
    AudioService::GetInstance()->foregroundSet_.clear();
    bool ret = AudioService::GetInstance()->MatchForegroundList(bundleName, uid);
    EXPECT_FALSE(ret);

    AudioService::GetInstance()->foregroundSet_.insert(bundleName);
    ret = AudioService::GetInstance()->MatchForegroundList(bundleName, uid);
    EXPECT_TRUE(ret);

    uid = 10;
    AudioService::GetInstance()->foregroundUidSet_.clear();
    ret = AudioService::GetInstance()->MatchForegroundList(bundleName, uid);
    EXPECT_TRUE(AudioService::GetInstance()->foregroundUidSet_.find(uid) !=
        AudioService::GetInstance()->foregroundUidSet_.end());
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test InForegroundList API
 * @tc.type  : FUNC
 * @tc.number: InForegroundList_001,
 * @tc.desc  : Test InForegroundList interface.
 */
HWTEST(AudioServiceUnitTest, InForegroundList_001, TestSize.Level1)
{
    uint32_t uid = 5;
    AudioService::GetInstance()->foregroundUidSet_.clear();
    bool ret = AudioService::GetInstance()->InForegroundList(uid);
    EXPECT_FALSE(ret);

    AudioService::GetInstance()->foregroundUidSet_.insert(uid);
    ret = AudioService::GetInstance()->InForegroundList(uid);
    EXPECT_TRUE(ret);
}

#ifdef SUPPORT_LOW_LATENCY
/**
 * @tc.name  : Test CheckBeforeRecordEndpointCreate API
 * @tc.type  : FUNC
 * @tc.number: CheckBeforeRecordEndpointCreate_001,
 * @tc.desc  : Test CheckBeforeRecordEndpointCreate interface.
 */
HWTEST(AudioServiceUnitTest, CheckBeforeRecordEndpointCreate_001, TestSize.Level1)
{
    bool isRecord = false;
    AudioService::GetInstance()->CheckBeforeRecordEndpointCreate(isRecord);

    isRecord = true;
    std::string endpointName = "test";
    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        123, clientConfig);
    EXPECT_NE(endpoint, nullptr);
    endpoint->clientConfig_.audioMode = AudioMode::AUDIO_MODE_RECORD;
    AudioService::GetInstance()->endpointList_[endpointName] = endpoint;
    AudioService::GetInstance()->CheckBeforeRecordEndpointCreate(isRecord);
}

/**
 * @tc.name  : Test CheckBeforeRecordEndpointCreate API
 * @tc.type  : FUNC
 * @tc.number: CheckBeforeRecordEndpointCreate_002,
 * @tc.desc  : Test CheckBeforeRecordEndpointCreate interface.
 */
HWTEST(AudioServiceUnitTest, CheckBeforeRecordEndpointCreate_002, TestSize.Level1)
{
    bool isRecord = true;
    std::string endpointName = "test";
    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        123, clientConfig);
    EXPECT_NE(endpoint, nullptr);
    endpoint->clientConfig_.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioService::GetInstance()->endpointList_[endpointName] = endpoint;
    AudioService::GetInstance()->CheckBeforeRecordEndpointCreate(isRecord);
}

/**
 * @tc.name  : Test NotifyStreamVolumeChanged API
 * @tc.type  : FUNC
 * @tc.number: NotifyStreamVolumeChanged_001
 * @tc.desc  : Test NotifyStreamVolumeChanged interface.
 */
HWTEST(AudioServiceUnitTest, NotifyStreamVolumeChanged_001, TestSize.Level1)
{
    float volume = 1.0f;
    AudioStreamType streamType = STREAM_MUSIC;

    std::string endpointName = "test";
    AudioService::GetInstance()->endpointList_.clear();
    AudioService::GetInstance()->endpointList_[endpointName] = nullptr;
    auto ret = AudioService::GetInstance()->NotifyStreamVolumeChanged(streamType, volume);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NotifyStreamVolumeChanged API
 * @tc.type  : FUNC
 * @tc.number: NotifyStreamVolumeChanged_002
 * @tc.desc  : Test NotifyStreamVolumeChanged interface.
 */
HWTEST(AudioServiceUnitTest, NotifyStreamVolumeChanged_002, TestSize.Level1)
{
    float volume = 1.0f;
    AudioStreamType streamType = STREAM_MUSIC;

    std::string endpointName = "test";
    AudioService::GetInstance()->endpointList_.clear();
    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        123, clientConfig);
    EXPECT_NE(endpoint, nullptr);
    AudioService::GetInstance()->endpointList_[endpointName] = endpoint;
    auto ret = AudioService::GetInstance()->NotifyStreamVolumeChanged(streamType, volume);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
 * @tc.name  : Test GetRendererBySessionID API
 * @tc.type  : FUNC
 * @tc.number: GetRendererBySessionId_001
 * @tc.desc  : Test GetRendererBySessionID interface.
 */
HWTEST(AudioServiceUnitTest, GetRendererBySessionId_001, TestSize.Level1)
{
    uint32_t sessionID = 10;
    AudioService::GetInstance()->allRendererMap_.clear();
    auto ret = AudioService::GetInstance()->GetRendererBySessionID(sessionID);
    EXPECT_EQ(ret, nullptr);

    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    EXPECT_NE(streamListenerHolder, nullptr);
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(rendererInServer, nullptr);
    std::weak_ptr<RendererInServer> server = rendererInServer;

    AudioService::GetInstance()->allRendererMap_[sessionID] = server;
    ret = AudioService::GetInstance()->GetRendererBySessionID(sessionID);
    EXPECT_NE(ret, nullptr);
}

#ifdef SUPPORT_LOW_LATENCY
/**
 * @tc.name  : Test SetNonInterruptMuteForProcess API
 * @tc.type  : FUNC
 * @tc.number: SetNonInterruptMuteForProcess_001
 * @tc.desc  : Test SetNonInterruptMuteForProcess interface.
 */
HWTEST(AudioServiceUnitTest, SetNonInterruptMuteForProcess_001, TestSize.Level1)
{
    uint32_t sessionId = 10;
    bool muteFlag = true;

    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_PLAYBACK;
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, AudioService::GetInstance());
    EXPECT_NE(audioprocess, nullptr);
    audioprocess->sessionId_ = 10;

    std::shared_ptr<AudioEndpointInner> endpoint = nullptr;
    AudioService::GetInstance()->linkedPairedList_.clear();
    AudioService::GetInstance()->linkedPairedList_.push_back(std::make_pair(audioprocess, endpoint));
    AudioService::GetInstance()->SetNonInterruptMuteForProcess(sessionId, muteFlag);
    EXPECT_EQ(AudioService::GetInstance()->linkedPairedList_.begin()->first->GetSessionId(), sessionId);

    sessionId = 0;
    AudioService::GetInstance()->SetNonInterruptMuteForProcess(sessionId, muteFlag);
}

/**
 * @tc.name  : Test SetNonInterruptMuteForProcess API
 * @tc.type  : FUNC
 * @tc.number: SetNonInterruptMuteForProcess_002
 * @tc.desc  : Test SetNonInterruptMuteForProcess interface.
 */
HWTEST(AudioServiceUnitTest, SetNonInterruptMuteForProcess_002, TestSize.Level1)
{
    uint32_t sessionId = 10;
    bool muteFlag = true;

    sptr<AudioProcessInServer> audioprocess = nullptr;

    std::shared_ptr<AudioEndpointInner> endpoint = nullptr;
    AudioService::GetInstance()->linkedPairedList_.clear();
    AudioService::GetInstance()->linkedPairedList_.push_back(std::make_pair(audioprocess, endpoint));
    AudioService::GetInstance()->SetNonInterruptMuteForProcess(sessionId, muteFlag);
    EXPECT_EQ(AudioService::GetInstance()->linkedPairedList_.begin()->first, nullptr);
}
#endif

/**
 * @tc.name  : Test SetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadMode_002
 * @tc.desc  : Test SetOffloadMode interface.
 */
HWTEST(AudioServiceUnitTest, SetOffloadMode_002, TestSize.Level1)
{
    uint32_t sessionId = 2;
    int32_t state = 1;
    bool isAppBack = true;
    AudioService::GetInstance()->allRendererMap_.clear();
    std::weak_ptr<RendererInServer> server;
    AudioService::GetInstance()->allRendererMap_[sessionId] = server;
    int32_t ret = AudioService::GetInstance()->SetOffloadMode(sessionId, state, isAppBack);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name  : Test SetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadMode_003
 * @tc.desc  : Test SetOffloadMode interface.
 */
HWTEST(AudioServiceUnitTest, SetOffloadMode_003, TestSize.Level1)
{
    uint32_t sessionId = 2;
    int32_t state = 1;
    bool isAppBack = true;
    AudioService::GetInstance()->allRendererMap_.clear();

    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    EXPECT_NE(streamListenerHolder, nullptr);
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> server =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(server, nullptr);

    bool isDirect = true;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    AudioService::GetInstance()->allRendererMap_[sessionId] = server;
    int32_t ret = AudioService::GetInstance()->SetOffloadMode(sessionId, state, isAppBack);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UnsetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: UnsetOffloadMode_002
 * @tc.desc  : Test UnsetOffloadMode interface.
 */
HWTEST(AudioServiceUnitTest, UnsetOffloadMode_002, TestSize.Level1)
{
    uint32_t sessionId = 10;
    AudioService::GetInstance()->allRendererMap_.clear();
    int ret = AudioService::GetInstance()->UnsetOffloadMode(sessionId);
    EXPECT_EQ(ret, ERR_INVALID_INDEX);

    std::weak_ptr<RendererInServer> server;
    AudioService::GetInstance()->allRendererMap_[sessionId] = server;
    ret = AudioService::GetInstance()->UnsetOffloadMode(sessionId);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test UnsetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: UnsetOffloadMode_003
 * @tc.desc  : Test UnsetOffloadMode interface.
 */
HWTEST(AudioServiceUnitTest, UnsetOffloadMode_003, TestSize.Level1)
{
    uint32_t sessionId = 10;
    AudioService::GetInstance()->allRendererMap_.clear();
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    EXPECT_NE(streamListenerHolder, nullptr);
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> server =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(server, nullptr);

    bool isDirect = true;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    AudioService::GetInstance()->allRendererMap_[sessionId] = server;
    auto ret = AudioService::GetInstance()->UnsetOffloadMode(sessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test UpdateAudioSinkState API
 * @tc.type  : FUNC
 * @tc.number: UpdateAudioSinkState_003
 * @tc.desc  : Test UpdateAudioSinkState interface.
 */
HWTEST(AudioServiceUnitTest, UpdateAudioSinkState_003, TestSize.Level1)
{
    uint32_t sinkId = 10;
    bool started = false;

    AudioService::GetInstance()->allRunningSinks_.clear();
    AudioService::GetInstance()->allRunningSinks_.insert(sinkId);
    AudioService::GetInstance()->UpdateAudioSinkState(sinkId, started);
    EXPECT_TRUE(AudioService::GetInstance()->allRunningSinks_.empty());

    uint32_t num = 5;
    AudioService::GetInstance()->allRunningSinks_.insert(num);
    AudioService::GetInstance()->allRunningSinks_.insert(sinkId);
    AudioService::GetInstance()->UpdateAudioSinkState(sinkId, started);
    EXPECT_FALSE(AudioService::GetInstance()->allRunningSinks_.empty());
}

/**
 * @tc.name  : Test UpdateSourceType API
 * @tc.type  : FUNC
 * @tc.number: UpdateSourceType_001
 * @tc.desc  : Test UPdateSourceType interface.
 */
HWTEST(AudioServiceUnitTest, UpdateSourceType_001, TestSize.Level1)
{
    SourceType sourceType = SOURCE_TYPE_WAKEUP;
    auto ret = AudioService::GetInstance()->UpdateSourceType(sourceType);
    EXPECT_EQ(ret, SUCCESS);

    sourceType = SOURCE_TYPE_MIC;
    ret = AudioService::GetInstance()->UpdateSourceType(sourceType);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test RestoreSession API
 * @tc.type  : FUNC
 * @tc.number: RestoreSession_001
 * @tc.desc  : Test RestoreSession interface.
 */
HWTEST(AudioServiceUnitTest, RestoreSession_001, TestSize.Level1)
{
    uint32_t sessionId = 10;
    RestoreInfo restoreInfo;
    std::weak_ptr<RendererInServer> rendererInServer;
    AudioService::GetInstance()->allRendererMap_.clear();
    AudioService::GetInstance()->allRendererMap_[sessionId] = rendererInServer;
    auto ret = AudioService::GetInstance()->RestoreSession(sessionId, restoreInfo);
    EXPECT_EQ(ret, RESTORE_ERROR);

    std::weak_ptr<CapturerInServer> capturerInServer;
    AudioService::GetInstance()->allRendererMap_.clear();
    AudioService::GetInstance()->allCapturerMap_.clear();
    AudioService::GetInstance()->allCapturerMap_[sessionId] = capturerInServer;
    ret = AudioService::GetInstance()->RestoreSession(sessionId, restoreInfo);
    EXPECT_EQ(ret, RESTORE_ERROR);
}

/**
 * @tc.name  : Test RestoreSession API
 * @tc.type  : FUNC
 * @tc.number: RestoreSession_002
 * @tc.desc  : Test RestoreSession interface.
 */
HWTEST(AudioServiceUnitTest, RestoreSession_002, TestSize.Level1)
{
    uint32_t sessionId = 10;
    RestoreInfo restoreInfo;
    AudioService::GetInstance()->allRendererMap_.clear();
    AudioService::GetInstance()->allCapturerMap_.clear();

    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_PLAYBACK;
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, AudioService::GetInstance());
    EXPECT_NE(audioprocess, nullptr);
    audioprocess->sessionId_ = 1;

    std::shared_ptr<AudioEndpointInner> endpoint = nullptr;
    AudioService::GetInstance()->linkedPairedList_.clear();
    AudioService::GetInstance()->linkedPairedList_.push_back(std::make_pair(audioprocess, endpoint));

    auto ret = AudioService::GetInstance()->RestoreSession(sessionId, restoreInfo);
    EXPECT_EQ(ret, RESTORE_ERROR);
}

/*
 * @tc.name  : Test RegisterMuteStateChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: RegisterMuteStateChangeCallback_001
 * @tc.desc  : Test RegisterMuteStateChangeCallback whether callback can invoke.
 */
HWTEST(AudioServiceUnitTest, RegisterMuteStateChangeCallback_001, TestSize.Level1)
{
    uint32_t sessionId = 10;
    bool muteFlag = false;
    MuteStateChangeCallbck muteStateChangeCallback = [&muteFlag](bool flag) { muteFlag = flag; };
    AudioService::GetInstance()->muteStateMap_.clear();
    AudioService::GetInstance()->muteStateCallbacks_.clear();
    AudioService::GetInstance()->muteStateCallbacks_[sessionId] = muteStateChangeCallback;
    AudioService::GetInstance()->RegisterMuteStateChangeCallback(sessionId, muteStateChangeCallback);

    AudioService::GetInstance()->muteStateMap_[sessionId] = true;
    AudioService::GetInstance()->RegisterMuteStateChangeCallback(sessionId, muteStateChangeCallback);
    EXPECT_EQ(muteFlag, true);
}

/*
 * @tc.name  : Test ForceStopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: ForceStopAudioStream_001
 * @tc.desc  : Test ForceStopAudioStream interface.
 */
HWTEST(AudioServiceUnitTest, ForceStopAudioStream_001, TestSize.Level1)
{
    StopAudioType stopAudioType = STOP_ALL;
    AudioService::GetInstance()->allRendererMap_.clear();
    AudioService::GetInstance()->allCapturerMap_.clear();
    AudioService::GetInstance()->linkedPairedList_.clear();
    auto ret = AudioService::GetInstance()->ForceStopAudioStream(stopAudioType);
    EXPECT_EQ(ret, SUCCESS);

    stopAudioType = STOP_RENDER;
    ret = AudioService::GetInstance()->ForceStopAudioStream(stopAudioType);
    EXPECT_EQ(ret, SUCCESS);

    stopAudioType = STOP_RECORD;
    ret = AudioService::GetInstance()->ForceStopAudioStream(stopAudioType);
    EXPECT_EQ(ret, SUCCESS);
}

#ifdef SUPPORT_LOW_LATENCY
/*
 * @tc.name  : Test ForceStopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: ForceStopAudioStream_002
 * @tc.desc  : Test ForceStopAudioStream interface.
 */
HWTEST(AudioServiceUnitTest, ForceStopAudioStream_002, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_PLAYBACK;
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, AudioService::GetInstance());
    EXPECT_NE(audioprocess, nullptr);
    audioprocess->sessionId_ = 1;

    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        123, config);
    EXPECT_NE(endpoint, nullptr);
    endpoint->clientConfig_.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioService::GetInstance()->linkedPairedList_.push_back(std::make_pair(audioprocess, endpoint));

    StopAudioType stopAudioType = STOP_ALL;
    auto ret = AudioService::GetInstance()->ForceStopAudioStream(stopAudioType);
    EXPECT_EQ(ret, SUCCESS);

    stopAudioType = STOP_RECORD;
    ret = AudioService::GetInstance()->ForceStopAudioStream(stopAudioType);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
 * @tc.name  : Test ConfigCoreServiceProvider API
 * @tc.type  : FUNC
 * @tc.number: ConfigCoreServiceProvider_001
 * @tc.desc  : Test ConfigCoreServiceProvider interface.
 */
HWTEST(AudioServiceUnitTest, ConfigCoreServiceProvider_001, TestSize.Level1)
{
    auto coreServiceHandler = CoreServiceHandler::GetInstance();
    sptr<ICoreServiceProviderIpc> provider = nullptr;
    auto result = coreServiceHandler.ConfigCoreServiceProvider(provider);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test AddThread API
 * @tc.type  : FUNC
 * @tc.number: AddThread_001
 * @tc.desc  : Test AddThread interface.
 */
HWTEST(AudioServiceUnitTest, AddThread_001, TestSize.Level1)
{
    AudioWorkgroup workgroup(1);
    int32_t tid = 10;
    ConcurrentTask::IntervalReply reply;
    reply.paramA = 1;
    int32_t result = workgroup.AddThread(tid);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test AddThread API
 * @tc.type  : FUNC
 * @tc.number: AddThread_002
 * @tc.desc  : Test AddThread interface.
 */
HWTEST(AudioServiceUnitTest, AddThread_002, TestSize.Level1)
{
    AudioWorkgroup workgroup(1);
    int32_t tid = 10;
    ConcurrentTask::IntervalReply reply;
    reply.paramA = -1;
    int32_t result = workgroup.AddThread(tid);
    EXPECT_NE(result, AUDIO_ERR);
}

/**
 * @tc.name  : Test AudioWorkgroup API
 * @tc.type  : FUNC
 * @tc.number: RemoveThread_001
 * @tc.desc  : Test AudioWorkgroup interface.
 */
HWTEST(AudioServiceUnitTest, RemoveThread_001, TestSize.Level1)
{
    AudioWorkgroup workgroup(1);
    int32_t tid = -1;
    ConcurrentTask::IntervalReply reply;
    reply.paramA = -1;
    int32_t result = workgroup.AddThread(tid);
    EXPECT_NE(result, AUDIO_ERR);
}

/**
 * @tc.name  : Test AudioWorkgroup API
 * @tc.type  : FUNC
 * @tc.number: RemoveThread_002
 * @tc.desc  : Test AudioWorkgroup interface.
 */
HWTEST(AudioServiceUnitTest, RemoveThread_002, TestSize.Level1)
{
    AudioWorkgroup workgroup(1);
    int32_t tid = -1;
    ConcurrentTask::IntervalReply reply;
    reply.paramA = 1;
    int32_t result = workgroup.AddThread(tid);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test AudioWorkgroup API
 * @tc.type  : FUNC
 * @tc.number: Start_001
 * @tc.desc  : Test AudioWorkgroup interface.
 */
HWTEST(AudioServiceUnitTest, Start_001, TestSize.Level1)
{
    AudioWorkgroup workgroup(1);
    int32_t result = workgroup.Start(100, 100);
    EXPECT_EQ(result, AUDIO_ERR);
}

/**
 * @tc.name  : Test AudioWorkgroup API
 * @tc.type  : FUNC
 * @tc.number: Start_002
 * @tc.desc  : Test AudioWorkgroup interface.
 */
HWTEST(AudioServiceUnitTest, Start_002, TestSize.Level1)
{
    AudioWorkgroup workgroup(1);
    int32_t result = workgroup.Start(100, 200);
    EXPECT_NE(result, AUDIO_OK);
}

/**
 * @tc.name  : Test AudioWorkgroup API
 * @tc.type  : FUNC
 * @tc.number: Start_003
 * @tc.desc  : Test AudioWorkgroup interface.
 */
HWTEST(AudioServiceUnitTest, Start_003, TestSize.Level1)
{
    AudioWorkgroup workgroup(1);
    int32_t result = workgroup.Start(200, 100);
    EXPECT_EQ(result, AUDIO_ERR);
}

/**
 * @tc.name  : Test AudioWorkgroup API
 * @tc.type  : FUNC
 * @tc.number: Stop_003
 * @tc.desc  : Test AudioWorkgroup interface.
 */
HWTEST(AudioServiceUnitTest, Stop_001, TestSize.Level1)
{
    AudioWorkgroup workgroup(1);
    int result = workgroup.Stop();
    if (RME::EndFrameFreq(0) != 0) {
        EXPECT_EQ(result, AUDIO_ERR);
    }
}

/**
 * @tc.name  : Test AudioWorkgroup API
 * @tc.type  : FUNC
 * @tc.number: Stop_002
 * @tc.desc  : Test AudioWorkgroup interface.
 */
HWTEST(AudioServiceUnitTest, Stop_002, TestSize.Level1)
{
    AudioWorkgroup workgroup(1);
    int result = workgroup.Stop();
    if (RME::EndFrameFreq(0) == 0) {
        EXPECT_EQ(result, AUDIO_OK);
    }
}

/**
 * @tc.name  : Test InRenderWhitelist API
 * @tc.type  : FUNC
 * @tc.number: InRenderWhitelist_001,
 * @tc.desc  : Test InRenderWhitelist interface.
 */
HWTEST(AudioServiceUnitTest, InRenderWhitelist_001, TestSize.Level1)
{
    std::string bundleName = "com.test";
    AudioService::GetInstance()->renderWhitelist_.clear();
    bool ret = AudioService::GetInstance()->InRenderWhitelist(bundleName);
    EXPECT_FALSE(ret);

    AudioService::GetInstance()->renderWhitelist_.insert(bundleName);
    ret = AudioService::GetInstance()->InRenderWhitelist(bundleName);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test SaveRenderWhitelist API
 * @tc.type  : FUNC
 * @tc.number: SaveRenderWhitelist_001,
 * @tc.desc  : Test SaveRenderWhitelist interface.
 */
HWTEST(AudioServiceUnitTest, SaveRenderWhitelist_001, TestSize.Level1)
{
    std::vector<std::string> list;
    list.resize(5);
    EXPECT_EQ(list.size(), 5);
    AudioService::GetInstance()->SaveRenderWhitelist(list);
    EXPECT_NE(AudioService::GetInstance()->renderWhitelist_.size(), 5);
}

/**
 * @tc.name  : Test UpdateSystemVolume API
 * @tc.type  : FUNC
 * @tc.number: UpdateSystemVolume_001,
 * @tc.desc  : Test UpdateSystemVolume interface.
 */
HWTEST(AudioServiceUnitTest, UpdateSystemVolume_001, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_ALARM;
    float volume = 0.5;

    // Act
    AudioService::GetInstance()->UpdateSystemVolume(streamType, volume);

    // Assert
    float expectedVolume = 0.0;
    EXPECT_NE(expectedVolume, AudioService::GetInstance()->musicOrVoipSystemVolume_);
}

/**
 * @tc.name  : Test UpdateSystemVolume API
 * @tc.type  : FUNC
 * @tc.number: UpdateSystemVolume_002,
 * @tc.desc  : Test UpdateSystemVolume interface.
 */
HWTEST(AudioServiceUnitTest, UpdateSystemVolume_002, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    float volume = 0.5;

    AudioService::GetInstance()->UpdateSystemVolume(streamType, volume);

    EXPECT_EQ(volume, AudioService::GetInstance()->musicOrVoipSystemVolume_);
}

/**
 * @tc.name  : Test UpdateSystemVolume API
 * @tc.type  : FUNC
 * @tc.number: UpdateSystemVolume_003,
 * @tc.desc  : Test UpdateSystemVolume interface.
 */
HWTEST(AudioServiceUnitTest, UpdateSystemVolume_003, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_VOICE_COMMUNICATION;
    float volume = 0.5;

    AudioService::GetInstance()->UpdateSystemVolume(streamType, volume);

    EXPECT_EQ(volume, AudioService::GetInstance()->musicOrVoipSystemVolume_);
}

/**
 * @tc.name  : Test SetSessionMuteState API
 * @tc.type  : FUNC
 * @tc.number: SetSessionMuteState_001,
 * @tc.desc  : Test SetSessionMuteState interface.
 */
HWTEST(AudioServiceUnitTest, SetSessionMuteState_001, TestSize.Level1)
{
    uint32_t sessionId = 1;
    bool insert = true;
    bool muteFlag = true;

    AudioService::GetInstance()->SetSessionMuteState(sessionId, insert, muteFlag);

    std::unique_lock<std::mutex> lock(AudioService::GetInstance()->muteStateMapMutex_);
    EXPECT_EQ(AudioService::GetInstance()->muteStateMap_[sessionId], muteFlag);
}

/**
 * @tc.name  : Test CleanAppUseNumMap API
 * @tc.type  : FUNC
 * @tc.number: CleanAppUseNumMap_001,
 * @tc.desc  : Test CleanAppUseNumMap interface.
 */
HWTEST(AudioServiceUnitTest, CleanAppUseNumMap_001, TestSize.Level1)
{
    int32_t appUid = 12345;
    AudioService::GetInstance()->appUseNumMap_[appUid] = 5;

    AudioService::GetInstance()->CleanAppUseNumMap(appUid);

    EXPECT_EQ(AudioService::GetInstance()->appUseNumMap_[appUid], 4);
}

/**
 * @tc.name  : Test CleanAppUseNumMap API
 * @tc.type  : FUNC
 * @tc.number: CleanAppUseNumMap_002,
 * @tc.desc  : Test CleanAppUseNumMap interface.
 */
HWTEST(AudioServiceUnitTest, CleanAppUseNumMap_002, TestSize.Level1)
{
    int32_t appUid = 12345;

    AudioService::GetInstance()->CleanAppUseNumMap(appUid);

    EXPECT_NE(AudioService::GetInstance()->appUseNumMap_.find(appUid),
              AudioService::GetInstance()->appUseNumMap_.end());
}

/**
 * @tc.name  : Test SetIncMaxRendererStreamCnt API
 * @tc.type  : FUNC
 * @tc.number: SetIncMaxRendererStreamCnt_001,
 * @tc.desc  : Test SetIncMaxRendererStreamCnt interface.
 */
HWTEST(AudioServiceUnitTest, SetIncMaxRendererStreamCnt_001, TestSize.Level1)
{
    int32_t initialCount = AudioService::GetInstance()->currentRendererStreamCnt_;

    AudioService::GetInstance()->SetIncMaxRendererStreamCnt(AUDIO_MODE_PLAYBACK);

    EXPECT_EQ(AudioService::GetInstance()->currentRendererStreamCnt_, initialCount + 1);
}

/**
 * @tc.name  : Test ShouldBeDualTone API
 * @tc.type  : FUNC
 * @tc.number: ShouldBeDualTone_001,
 * @tc.desc  : Test ShouldBeDualTone interface.
 */
HWTEST(AudioServiceUnitTest, ShouldBeDualTone_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;

    EXPECT_FALSE(AudioService::GetInstance()->ShouldBeDualTone(config));

    config.rendererInfo.streamUsage = STREAM_USAGE_RINGTONE;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    EXPECT_FALSE(AudioService::GetInstance()->ShouldBeDualTone(config));
}

/**
 * @tc.name  : Test ShouldBeDualTone API
 * @tc.type  : FUNC
 * @tc.number: ShouldBeDualTone_002,
 * @tc.desc  : Test ShouldBeDualTone interface.
 */
HWTEST(AudioServiceUnitTest, ShouldBeDualTone_002, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.rendererInfo.streamUsage = STREAM_USAGE_RINGTONE;
    config.audioMode = AUDIO_MODE_RECORD;

    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    EXPECT_FALSE(AudioService::GetInstance()->ShouldBeDualTone(config));
}

/**
 * @tc.name  : Test ShouldBeDualTone API
 * @tc.type  : FUNC
 * @tc.number: ShouldBeDualTone_003,
 * @tc.desc  : Test ShouldBeDualTone interface.
 */
HWTEST(AudioServiceUnitTest, ShouldBeDualTone_003, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.rendererInfo.streamUsage = STREAM_USAGE_RINGTONE;
    config.audioMode = AUDIO_MODE_PLAYBACK;

    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = DEVICE_TYPE_WIRED_HEADSET;

    EXPECT_FALSE(AudioService::GetInstance()->ShouldBeDualTone(config));
}

/**
 * @tc.name  : Test ShouldBeDualTone API
 * @tc.type  : FUNC
 * @tc.number: ShouldBeDualTone_004,
 * @tc.desc  : Test ShouldBeDualTone interface.
 */
HWTEST(AudioServiceUnitTest, ShouldBeDualTone_004, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.rendererInfo.streamUsage = STREAM_USAGE_RINGTONE;
    config.audioMode = AUDIO_MODE_PLAYBACK;

    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = static_cast<DeviceType>(999); // 未知设备类型

    EXPECT_FALSE(AudioService::GetInstance()->ShouldBeDualTone(config));
}

/**
 * @tc.name  : Test GetDeviceInfoForProcess API
 * @tc.type  : FUNC
 * @tc.number: GetDeviceInfoForProcess_001,
 * @tc.desc  : Test GetDeviceInfoForProcess interface.
 */
HWTEST(AudioServiceUnitTest, GetDeviceInfoForProcess_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.originalSessionId = 1;
    config.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    config.streamInfo.samplingRate = SAMPLE_RATE_16000;
    bool reload = false;
    AudioStreamInfo info;
    AudioDeviceDescriptor deviceInfo = AudioService::GetInstance()->GetDeviceInfoForProcess(config, info, reload);

    EXPECT_NE(deviceInfo.deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(deviceInfo.isLowLatencyDevice_, false);
    EXPECT_NE(deviceInfo.audioStreamInfo_.size(), 1);
}

/**
 * @tc.name  : Test GetDeviceInfoForProcess API
 * @tc.type  : FUNC
 * @tc.number: GetDeviceInfoForProcess_002,
 * @tc.desc  : Test GetDeviceInfoForProcess interface.
 */
HWTEST(AudioServiceUnitTest, GetDeviceInfoForProcess_002, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.originalSessionId = 1;
    config.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    config.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    config.streamInfo.samplingRate = SAMPLE_RATE_16000;
    bool reload = false;
    AudioStreamInfo info;
    AudioDeviceDescriptor deviceInfo = AudioService::GetInstance()->GetDeviceInfoForProcess(config, info, reload);

    EXPECT_NE(deviceInfo.deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(deviceInfo.isLowLatencyDevice_, false);
    EXPECT_EQ(deviceInfo.a2dpOffloadFlag_, 0);
    EXPECT_NE(deviceInfo.audioStreamInfo_.size(), 1);
    EXPECT_EQ(deviceInfo.deviceName_, "mmap_device");
}

/**
 * @tc.name  : Test GetDeviceInfoForProcess API
 * @tc.type  : FUNC
 * @tc.number: GetDeviceInfoForProcess_003,
 * @tc.desc  : Test GetDeviceInfoForProcess interface.
 */
HWTEST(AudioServiceUnitTest, GetDeviceInfoForProcess_003, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.originalSessionId = 1;
    config.audioMode = AUDIO_MODE_RECORD;
    bool reload = false;
    AudioStreamInfo info;
    AudioDeviceDescriptor deviceInfo = AudioService::GetInstance()->GetDeviceInfoForProcess(config, info, reload);

    EXPECT_EQ(deviceInfo.deviceId_, 1);
    EXPECT_EQ(deviceInfo.networkId_, LOCAL_NETWORK_ID);
    EXPECT_EQ(deviceInfo.deviceRole_, INPUT_DEVICE);
    EXPECT_EQ(deviceInfo.deviceType_, DEVICE_TYPE_MIC);
    EXPECT_NE(deviceInfo.audioStreamInfo_.size(), 1);

    EXPECT_EQ(deviceInfo.deviceName_, "mmap_device");
}

/**
 * @tc.name  : Test InitAllDupBuffer API
 * @tc.type  : FUNC
 * @tc.number: InitAllDupBuffer_001,
 * @tc.desc  : Test InitAllDupBuffer interface.
 */
HWTEST(AudioServiceUnitTest, InitAllDupBuffer_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    int32_t innerCapId = 1;

    std::weak_ptr<RendererInServer> server;
    std::unique_lock<std::mutex> lock(audioService->rendererMapMutex_);
    AudioService::GetInstance()->filteredRendererMap_[innerCapId].push_back(server);
    lock.unlock();

    AudioService::GetInstance()->InitAllDupBuffer(innerCapId);
}

/**
 * @tc.name  : Test RenderersCheckForAudioWorkgroup API
 * @tc.type  : FUNC
 * @tc.number: RenderersCheckForAudioWorkgroup_001,
 * @tc.desc  : Test RenderersCheckForAudioWorkgroup interface.
 */
HWTEST(AudioServiceUnitTest, RenderersCheckForAudioWorkgroup_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->RenderersCheckForAudioWorkgroup(1);
    EXPECT_FALSE(AudioResourceService::GetInstance()->IsProcessInWorkgroup(1));
    EXPECT_FALSE(AudioResourceService::GetInstance()->IsProcessHasSystemPermission(1));

    audioService->RenderersCheckForAudioWorkgroup(-1);
    EXPECT_FALSE(AudioResourceService::GetInstance()->IsProcessInWorkgroup(-1));
    EXPECT_FALSE(AudioResourceService::GetInstance()->IsProcessHasSystemPermission(-1));
}

/**
 * @tc.name  : Test GetSystemVolume API
 * @tc.type  : FUNC
 * @tc.number: GetSystemVolume_001,
 * @tc.desc  : Test GetSystemVolume interface.
 */
HWTEST(AudioServiceUnitTest, GetSystemVolume_001, TestSize.Level1)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->musicOrVoipSystemVolume_ = 0.5;
    float volume = 0.0;
    volume = audioService->GetSystemVolume();
    EXPECT_EQ(volume, 0.5);

    audioService->musicOrVoipSystemVolume_ = 1.0;
    volume = audioService->GetSystemVolume();
    EXPECT_EQ(volume, 1.0);

    audioService->musicOrVoipSystemVolume_ = 0.0;
    volume = audioService->GetSystemVolume();
    EXPECT_EQ(volume, 0.0);
}

/**
 * @tc.name  : Test LinkProcessToEndpoint API
 * @tc.type  : FUNC
 * @tc.number: LinkProcessToEndpoint_001,
 * @tc.desc  : Test LinkProcessToEndpoint interface.
 */
HWTEST(AudioServiceUnitTest, LinkProcessToEndpoint_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_PLAYBACK;
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, AudioService::GetInstance());
    EXPECT_NE(audioprocess, nullptr);
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        123, config);
    EXPECT_NE(AudioService::GetInstance()->LinkProcessToEndpoint(audioprocess, endpoint), SUCCESS);
}

/**
 * @tc.name  : Test UpdateForegroundState API
 * @tc.type  : FUNC
 * @tc.number: UpdateForegroundState_001,
 * @tc.desc  : Test UpdateForegroundState interface.
 */
HWTEST(AudioServiceUnitTest, UpdateForegroundState_001, TestSize.Level1)
{
    uint32_t appTokenId = 12345;
    bool isActive = true;
    bool result = AudioService::GetInstance()->UpdateForegroundState(appTokenId, isActive);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test UpdateForegroundState API
 * @tc.type  : FUNC
 * @tc.number: UpdateForegroundState_002,
 * @tc.desc  : Test UpdateForegroundState interface.
 */
HWTEST(AudioServiceUnitTest, UpdateForegroundState_002, TestSize.Level1)
{
    uint32_t appTokenId = -1;
    bool isActive = true;
    bool result = AudioService::GetInstance()->UpdateForegroundState(appTokenId, isActive);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test DumpForegroundList API
 * @tc.type  : FUNC
 * @tc.number: DumpForegroundList_001,
 * @tc.desc  : Test DumpForegroundList interface.
 */
HWTEST(AudioServiceUnitTest, DumpForegroundList_001, TestSize.Level1)
{
    std::string dumpString;
    AudioService::GetInstance()->DumpForegroundList(dumpString);
    EXPECT_NE(dumpString, "DumpForegroundList:\n");
}

/**
 * @tc.name  : Test ConfigCoreServiceProvider API
 * @tc.type  : FUNC
 * @tc.number: ConfigCoreServiceProvider_002,
 * @tc.desc  : Test ConfigCoreServiceProvider interface.
 */
HWTEST(AudioServiceUnitTest, ConfigCoreServiceProvider_002, TestSize.Level1)
{
    auto coreServiceHandler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, samgr);
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    int32_t ret = coreServiceHandler.ConfigCoreServiceProvider(coreServiceProvider);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ConfigCoreServiceProvider API
 * @tc.type  : FUNC
 * @tc.number: ConfigCoreServiceProvider_003,
 * @tc.desc  : Test ConfigCoreServiceProvider interface.
 */
HWTEST(AudioServiceUnitTest, ConfigCoreServiceProvider_003, TestSize.Level1)
{
    auto coreServiceHandler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, samgr);
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    coreServiceHandler.ConfigCoreServiceProvider(coreServiceProvider); // Set the provider
    int32_t ret = coreServiceHandler.ConfigCoreServiceProvider(coreServiceProvider);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

} // namespace AudioStandard
} // namespace OHOS