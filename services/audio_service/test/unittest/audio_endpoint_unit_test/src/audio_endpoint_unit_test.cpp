/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_endpoint_unit_test.h"

#include "accesstoken_kit.h"
#include "audio_device_info.h"
#include "audio_endpoint.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_process_config.h"
#include "audio_server.h"
#include "audio_service.h"
#include "audio_stream_info.h"
#include "policy_handler.h"
#include "audio_endpoint.cpp"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t DEFAULT_STREAM_ID = 10;
constexpr uint64_t AUDIO_ENDPOINT_ID = 123;

void AudioEndpointUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioEndpointUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioEndpointUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioEndpointUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

static std::shared_ptr<AudioEndpointInner> CreateEndpointInner(AudioEndpoint::EndpointType type, uint64_t id,
    const AudioProcessConfig &clientConfig, const AudioDeviceDescriptor &deviceInfo, AudioStreamInfo &streamInfo)
{
    std::shared_ptr<AudioEndpointInner> audioEndpoint =
        std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    CHECK_AND_RETURN_RET_LOG(audioEndpoint != nullptr, nullptr, "Create AudioEndpoint failed.");

    if (!audioEndpoint->Config(deviceInfo, streamInfo)) {
        audioEndpoint = nullptr;
    }
    return audioEndpoint;
}

static std::shared_ptr<AudioEndpointInner> CreateInputEndpointInner(AudioEndpoint::EndpointType type)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpoint =
        std::make_shared<AudioEndpointInner>(type, AUDIO_ENDPOINT_ID, config);
    if (!audioEndpoint->Config(deviceInfo, audioStreamInfo)) {
        audioEndpoint = nullptr;
    }
    return audioEndpoint;
}

static std::shared_ptr<AudioEndpointInner> CreateOutputEndpointInner(AudioEndpoint::EndpointType type)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(type, AUDIO_ENDPOINT_ID, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);
    EXPECT_NE(HDI_INVALID_ID, audioEndpointInner->fastRenderId_);

    return audioEndpointInner;
}

static AudioProcessConfig InitServerProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

static sptr<AudioProcessInServer> CreateAudioProcessInServer()
{
    AudioService *audioServicePtr = AudioService::GetInstance();
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.samplingRate = SAMPLE_RATE_48000;
    audioStreamInfo.channels = STEREO;
    audioStreamInfo.channelLayout = CH_LAYOUT_STEREO;
    AudioProcessConfig serverConfig = InitServerProcessConfig();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(serverConfig, audioServicePtr);
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    processStream->ConfigProcessBuffer(totalSizeInFrame, spanSizeInFrame, audioStreamInfo, buffer);
    return processStream;
}

/**
 * @tc.name  : Test CreateEndpoint API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointCreateEndpoint_001
 * @tc.desc  : Test CreateEndpoint interface, networkId is LOCAL_NETWORK_ID.
 */
HWTEST_F(AudioEndpointUnitTest, AudioEndpointCreateEndpoint_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpoint =
        AudioEndpoint::CreateEndpoint(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpoint);
}

/**
 * @tc.name  : Test CreateEndpoint API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointCreateEndpoint_002
 * @tc.desc  : Test CreateEndpoint interface.
 */
HWTEST_F(AudioEndpointUnitTest, CreateEndpoint_002, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_U8, STEREO, CH_LAYOUT_STEREO };

    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S24LE;

    audioEndpointInner = CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S32LE;

    audioEndpointInner = CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    audioStreamInfo.format = AudioSampleFormat::INVALID_WIDTH;

    audioEndpointInner = CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioEndpointInner = CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);
}

/**
 * @tc.name  : Test CreateEndpoint API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointCreateEndpoint_002
 * @tc.desc  : Test CreateEndpoint interface, networkId is LOCAL_NETWORK_ID, deviceRole is INPUT_DEVICE.
 */
HWTEST_F(AudioEndpointUnitTest, AudioEndpointCreateEndpoint_002, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpoint =
        AudioEndpoint::CreateEndpoint(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpoint);
}

/**
 * @tc.name  : Test EnableFastInnerCap API
 * @tc.type  : FUNC
 * @tc.number: AudioEnableFastInnerCap_001
 * @tc.desc  : Test EnableFastInnerCap interface.
 */
HWTEST_F(AudioEndpointUnitTest, AudioEnableFastInnerCap_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpoint =
        AudioEndpoint::CreateEndpoint(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpoint);

    int32_t ret = audioEndpoint->EnableFastInnerCap(1);
    EXPECT_EQ(SUCCESS, ret);

    audioEndpoint->Release();
}

/*
 * @tc.name  : Test EnableFastInnerCap API
 * @tc.type  : FUNC
 * @tc.number: AudioEnableFastInnerCap_002
 * @tc.desc  : Test EnableFastInnerCap interface
 */
HWTEST_F(AudioEndpointUnitTest, AudioEnableFastInnerCap_002, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    deviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string ret = audioEndpointInner->GenerateEndpointKey(deviceInfo, 1);
    EXPECT_NE("", ret);

    deviceInfo.deviceType_ = DEVICE_TYPE_INVALID;
    audioEndpointInner->GenerateEndpointKey(deviceInfo, 1);
    EXPECT_NE("", ret);

    auto &info = audioEndpointInner->fastCaptureInfos_[1];
    info.isInnerCapEnabled = true;
    audioEndpointInner->deviceInfo_.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    int32_t result = audioEndpointInner->EnableFastInnerCap(1);
    EXPECT_EQ(SUCCESS, result);

    result = audioEndpointInner->DisableFastInnerCap();
    EXPECT_EQ(SUCCESS, result);

    result = audioEndpointInner->DisableFastInnerCap();
    EXPECT_EQ(SUCCESS, result);

    result = audioEndpointInner->DisableFastInnerCap();
    EXPECT_EQ(SUCCESS, result);
}

/*
 * @tc.name  : Test HandleZeroVolumeCheckEvent API
 * @tc.type  : FUNC
 * @tc.number: HandleZeroVolumeCheckEvent_001
 * @tc.desc  : Test HandleZeroVolumeCheckEvent interface
 */
HWTEST_F(AudioEndpointUnitTest, HandleZeroVolumeCheckEvent_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);
    EXPECT_NE(HDI_INVALID_ID, audioEndpointInner->fastRenderId_);

    audioEndpointInner->zeroVolumeStopDevice_ = true;
    audioEndpointInner->HandleZeroVolumeCheckEvent();
    EXPECT_TRUE(audioEndpointInner->zeroVolumeStopDevice_);

    audioEndpointInner->zeroVolumeStopDevice_ = false;
    audioEndpointInner->HandleZeroVolumeCheckEvent();
    EXPECT_FALSE(audioEndpointInner->zeroVolumeStopDevice_);

    audioEndpointInner->zeroVolumeStopDevice_ = false;
    audioEndpointInner->delayStopTimeForZeroVolume_ = 0;
    audioEndpointInner->isStarted_ = false;
    audioEndpointInner->HandleZeroVolumeCheckEvent();
    EXPECT_TRUE(audioEndpointInner->zeroVolumeStopDevice_);

    audioEndpointInner->zeroVolumeStopDevice_ = false;
    audioEndpointInner->delayStopTimeForZeroVolume_ = 0;
    audioEndpointInner->isStarted_ = true;
    audioEndpointInner->HandleZeroVolumeCheckEvent();
    EXPECT_TRUE(audioEndpointInner->zeroVolumeStopDevice_);

    audioEndpointInner->zeroVolumeStopDevice_ = false;
    audioEndpointInner->delayStopTimeForZeroVolume_ = 0;
    audioEndpointInner->isStarted_ = true;
    HdiAdapterManager::GetInstance().ReleaseId(audioEndpointInner->fastRenderId_);
    audioEndpointInner->HandleZeroVolumeCheckEvent();
    EXPECT_TRUE(audioEndpointInner->zeroVolumeStopDevice_);
}

/*
 * @tc.name  : Test ZeroVolumeCheck API
 * @tc.type  : FUNC
 * @tc.number: ZeroVolumeCheck_001
 * @tc.desc  : Test ZeroVolumeCheck interface
 */
HWTEST_F(AudioEndpointUnitTest, ZeroVolumeCheck_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->zeroVolumeStopDevice_ = true;
    audioEndpointInner->ZeroVolumeCheck(0);

    audioEndpointInner->zeroVolumeStopDevice_ = false;
    audioEndpointInner->isVolumeAlreadyZero_ = true;
    audioEndpointInner->ZeroVolumeCheck(0);

    audioEndpointInner->zeroVolumeStopDevice_ = false;
    audioEndpointInner->isVolumeAlreadyZero_ = false;
    audioEndpointInner->ZeroVolumeCheck(0);
    EXPECT_TRUE(audioEndpointInner->isVolumeAlreadyZero_);

    audioEndpointInner->zeroVolumeStopDevice_ = false;
    audioEndpointInner->ZeroVolumeCheck(1);
    EXPECT_FALSE(audioEndpointInner->isVolumeAlreadyZero_);

    audioEndpointInner->zeroVolumeStopDevice_ = true;
    audioEndpointInner->isStarted_ = true;
    audioEndpointInner->ZeroVolumeCheck(1);
    EXPECT_FALSE(audioEndpointInner->isVolumeAlreadyZero_);

    audioEndpointInner->zeroVolumeStopDevice_ = true;
    audioEndpointInner->isStarted_ = false;
    audioEndpointInner->ZeroVolumeCheck(1);
    EXPECT_FALSE(audioEndpointInner->isVolumeAlreadyZero_);

    audioEndpointInner->zeroVolumeStopDevice_ = true;
    audioEndpointInner->isStarted_ = false;
    HdiAdapterManager::GetInstance().ReleaseId(audioEndpointInner->fastRenderId_);
    audioEndpointInner->ZeroVolumeCheck(1);
    EXPECT_FALSE(audioEndpointInner->isVolumeAlreadyZero_);
}

/*
 * @tc.name  : Test KeepWorkloopRunning API
 * @tc.type  : FUNC
 * @tc.number: KeepWorkloopRunning_001
 * @tc.desc  : Test KeepWorkloopRunning interface
 */
HWTEST_F(AudioEndpointUnitTest, KeepWorkloopRunning_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->endpointStatus_.store(AudioEndpoint::EndpointStatus::RUNNING);
    EXPECT_TRUE(audioEndpointInner->KeepWorkloopRunning());

    audioEndpointInner->endpointStatus_.store(AudioEndpoint::EndpointStatus::UNLINKED);
    EXPECT_FALSE(audioEndpointInner->KeepWorkloopRunning());

    audioEndpointInner->endpointStatus_.store(AudioEndpoint::EndpointStatus::STARTING);
    EXPECT_FALSE(audioEndpointInner->KeepWorkloopRunning());

    audioEndpointInner->endpointStatus_.store(AudioEndpoint::EndpointStatus::STOPPING);
    EXPECT_FALSE(audioEndpointInner->KeepWorkloopRunning());

    audioEndpointInner->endpointStatus_.store(static_cast<AudioEndpoint::EndpointStatus>(10));
    EXPECT_FALSE(audioEndpointInner->KeepWorkloopRunning());

    audioEndpointInner->isDeviceRunningInIdel_ = false;
    audioEndpointInner->endpointStatus_.store(AudioEndpoint::EndpointStatus::IDEL);
    EXPECT_FALSE(audioEndpointInner->KeepWorkloopRunning());

    audioEndpointInner->isDeviceRunningInIdel_ = true;
    audioEndpointInner->endpointStatus_.store(AudioEndpoint::EndpointStatus::IDEL);
    EXPECT_TRUE(audioEndpointInner->KeepWorkloopRunning());

    audioEndpointInner->delayStopTime_ = 0;
    audioEndpointInner->endpointStatus_.store(AudioEndpoint::EndpointStatus::IDEL);
    EXPECT_FALSE(audioEndpointInner->KeepWorkloopRunning());
}

/*
 * @tc.name  : Test CheckUpdateState API
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateState_001
 * @tc.desc  : Test CheckUpdateState interface
 */
HWTEST_F(AudioEndpointUnitTest, CheckUpdateState_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    char frame[] = {'8', '8', '8', '8', '8', '8', '8', '8'};
    audioEndpointInner->startUpdate_ = false;
    audioEndpointInner->CheckUpdateState(frame, 64);
    audioEndpointInner->startUpdate_ = true;
    audioEndpointInner->renderFrameNum_ = 0;
    audioEndpointInner->CheckUpdateState(frame, 64);
    EXPECT_EQ(1, audioEndpointInner->renderFrameNum_);

    audioEndpointInner->startUpdate_ = true;
    audioEndpointInner->renderFrameNum_ = 39;
    audioEndpointInner->last10FrameStartTime_ = 100;
    audioEndpointInner->lastGetMaxAmplitudeTime_ = 0;
    audioEndpointInner->CheckUpdateState(frame, 64);
    EXPECT_EQ(0, audioEndpointInner->renderFrameNum_);

    audioEndpointInner->startUpdate_ = true;
    audioEndpointInner->renderFrameNum_ = 39;
    audioEndpointInner->lastGetMaxAmplitudeTime_ = 100;
    audioEndpointInner->last10FrameStartTime_ = 0;
    audioEndpointInner->CheckUpdateState(frame, 64);
    EXPECT_EQ(0, audioEndpointInner->renderFrameNum_);
}

/*
 * @tc.name  : Test ProcessToDupStream API
 * @tc.type  : FUNC
 * @tc.number: CheckProcessToDupStream_001
 * @tc.desc  : Test ProcessToDupStream interface
 */
HWTEST_F(AudioEndpointUnitTest, CheckProcessToDupStream_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo streamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, streamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->endpointType_ = AudioEndpoint::EndpointType::TYPE_INVALID;
    std::vector<AudioStreamData> audioDataList;
    AudioStreamData audioStreamInfo = {};
    audioEndpointInner->ProcessToDupStream(audioDataList, audioStreamInfo, 1);

    audioEndpointInner->endpointType_ = AudioEndpoint::EndpointType::TYPE_VOIP_MMAP;
    audioDataList.push_back(audioStreamInfo);
    audioEndpointInner->ProcessToDupStream(audioDataList, audioStreamInfo, 1);
}

/*
 * @tc.name  : Test GetFastSink API
 * @tc.type  : FUNC
 * @tc.number: GetFastSink_001
 * @tc.desc  : Test GetFastSink interface
 */
HWTEST_F(AudioEndpointUnitTest, GetFastSink_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    deviceInfo.networkId_ = REMOTE_NETWORK_ID;
    auto ret = audioEndpointInner->GetFastSink(deviceInfo, AudioEndpoint::TYPE_MMAP);
    EXPECT_NE(nullptr, ret);

    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    deviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceInfo.a2dpOffloadFlag_ = A2DP_NOT_OFFLOAD;
    ret = audioEndpointInner->GetFastSink(deviceInfo, AudioEndpoint::TYPE_INVALID);
    EXPECT_NE(nullptr, ret);

    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    deviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceInfo.a2dpOffloadFlag_ = A2DP_OFFLOAD;
    ret = audioEndpointInner->GetFastSink(deviceInfo, AudioEndpoint::TYPE_INVALID);
    EXPECT_EQ(nullptr, ret);
}

/*
 * @tc.name  : Test AudioEndpoint API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointMix_001
 * @tc.desc  : Test AudioEndpointInner interface
 */
HWTEST_F(AudioEndpointUnitTest, AudioEndpointMix_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_MMAP);
    sptr<AudioProcessInServer> processStream = CreateAudioProcessInServer();
    sptr<AudioProcessInServer> newpProcessStream = CreateAudioProcessInServer();
    EXPECT_NE(nullptr, audioEndpointInner);

    bool result = audioEndpointInner->UnlinkProcessStream(processStream);
    EXPECT_FALSE(result);

    int32_t ret = audioEndpointInner->OnUpdateHandleInfo(processStream);
    EXPECT_NE(SUCCESS, ret);

    ret = audioEndpointInner->LinkProcessStream(processStream);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioEndpointInner->OnUpdateHandleInfo(processStream);
    EXPECT_EQ(SUCCESS, ret);

    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    result = audioEndpointInner->Config(deviceInfo, audioStreamInfo);
    EXPECT_FALSE(result);

    processStream->SetInnerCapState(true, 1);
    result = audioEndpointInner->ShouldInnerCap(1);
    EXPECT_TRUE(result);

    processStream->SetInnerCapState(false, 1);
    result = audioEndpointInner->ShouldInnerCap(1);
    EXPECT_FALSE(result);

    result = audioEndpointInner->UnlinkProcessStream(newpProcessStream);
    EXPECT_EQ(SUCCESS, ret);

    result = audioEndpointInner->UnlinkProcessStream(processStream);
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * @tc.name  : Test AudioEndpoint API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointMix_002
 * @tc.desc  : Test AudioEndpointInner interface
 */
HWTEST_F(AudioEndpointUnitTest, AudioEndpointMix_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_MMAP);
    sptr<AudioProcessInServer> processStream = CreateAudioProcessInServer();
    EXPECT_NE(nullptr, audioEndpointInner);

    int32_t ret = audioEndpointInner->LinkProcessStream(processStream);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioEndpointInner->OnPause(processStream);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioEndpointInner->OnStart(processStream);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioEndpointInner->OnStart(processStream);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioEndpointInner->OnPause(processStream);
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * @tc.name  : Test AudioEndpoint API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointMix_003
 * @tc.desc  : Test AudioEndpointInner interface
 */
HWTEST_F(AudioEndpointUnitTest, AudioEndpointMix_003, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    sptr<AudioProcessInServer> processStream = CreateAudioProcessInServer();
    EXPECT_NE(nullptr, audioEndpointInner);

    int32_t ret = audioEndpointInner->LinkProcessStream(processStream);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioEndpointInner->OnPause(processStream);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioEndpointInner->OnStart(processStream);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioEndpointInner->OnStart(processStream);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioEndpointInner->OnPause(processStream);
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * @tc.name  : Test HandleStartDeviceFailed API
 * @tc.type  : FUNC
 * @tc.number: HandleStartDeviceFailed_001
 * @tc.desc  : Test HandleStartDeviceFailed interface
 */
HWTEST_F(AudioEndpointUnitTest, HandleStartDeviceFailed_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_MMAP);
    sptr<AudioProcessInServer> processStream = CreateAudioProcessInServer();
    sptr<AudioProcessInServer> newpProcessStream = CreateAudioProcessInServer();
    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->HandleStartDeviceFailed();
    EXPECT_EQ(AudioEndpoint::EndpointStatus::UNLINKED, audioEndpointInner->endpointStatus_);

    int32_t ret = audioEndpointInner->LinkProcessStream(processStream);
    EXPECT_EQ(SUCCESS, ret);

    audioEndpointInner->LinkProcessStream(newpProcessStream);
    EXPECT_EQ(SUCCESS, ret);

    audioEndpointInner->HandleStartDeviceFailed();
    EXPECT_EQ(AudioEndpoint::EndpointStatus::IDEL, audioEndpointInner->endpointStatus_);
    auto &info = audioEndpointInner->fastCaptureInfos_[1];
    info.isInnerCapEnabled = true;
    EXPECT_TRUE(audioEndpointInner->StartDevice());

    EXPECT_TRUE(audioEndpointInner->StopDevice());

    std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(
        audioEndpointInner->fastCaptureId_);
    ASSERT_NE(nullptr, source);
    source->DeInit();
    HdiAdapterManager::GetInstance().ReleaseId(audioEndpointInner->fastCaptureId_);
    EXPECT_FALSE(audioEndpointInner->StartDevice());
}

/*
 * @tc.name  : Test EndpointWorkLoopFuc API
 * @tc.type  : FUNC
 * @tc.number: EndpointWorkLoopFuc_001
 * @tc.desc  : Test EndpointWorkLoopFuc interface
 */
HWTEST_F(AudioEndpointUnitTest, EndpointWorkLoopFuc_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    sptr<AudioProcessInServer> processStream = CreateAudioProcessInServer();
    sptr<AudioProcessInServer> newpProcessStream = CreateAudioProcessInServer();
    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->HandleStartDeviceFailed();
    EXPECT_EQ(AudioEndpoint::EndpointStatus::UNLINKED, audioEndpointInner->endpointStatus_);

    int32_t ret = audioEndpointInner->LinkProcessStream(processStream);
    EXPECT_EQ(SUCCESS, ret);

    audioEndpointInner->LinkProcessStream(newpProcessStream);
    EXPECT_EQ(SUCCESS, ret);

    audioEndpointInner->HandleStartDeviceFailed();
    EXPECT_EQ(AudioEndpoint::EndpointStatus::IDEL, audioEndpointInner->endpointStatus_);

    EXPECT_TRUE(audioEndpointInner->StartDevice());
}

/*
 * @tc.name  : Test DelayStopDevice API
 * @tc.type  : FUNC
 * @tc.number: DelayStopDevice_001
 * @tc.desc  : Test DelayStopDevice interface
 */
HWTEST_F(AudioEndpointUnitTest, DelayStopDevice_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_MMAP);
    sptr<AudioProcessInServer> processStream = CreateAudioProcessInServer();
    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->delayStopTime_ = 0;
    int32_t ret = audioEndpointInner->LinkProcessStream(processStream);
    EXPECT_EQ(SUCCESS, ret);

    EXPECT_TRUE(audioEndpointInner->DelayStopDevice());

    std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(
        audioEndpointInner->fastCaptureId_);
    ASSERT_NE(nullptr, source);
    source->DeInit();
    HdiAdapterManager::GetInstance().ReleaseId(audioEndpointInner->fastCaptureId_);
    auto &info = audioEndpointInner->fastCaptureInfos_[1];
    info.isInnerCapEnabled = true;
    EXPECT_FALSE(audioEndpointInner->DelayStopDevice());

    audioEndpointInner->deviceInfo_.deviceRole_ = OUTPUT_DEVICE;
    EXPECT_FALSE(audioEndpointInner->DelayStopDevice());
}

/*
 * @tc.name  : Test GetEndpointName API
 * @tc.type  : FUNC
 * @tc.number: GetEndpointName_001
 * @tc.desc  : Test GetEndpointName interface
 */
HWTEST_F(AudioEndpointUnitTest, GetEndpointName_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_MMAP);
    EXPECT_NE(nullptr, audioEndpointInner);
    audioEndpointInner->GetEndpointName();
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t ret = audioEndpointInner->SetVolume(streamType, 0.0f);
    EXPECT_EQ(0, ret);
    std::string dumpString = "";
    audioEndpointInner->Dump(dumpString);
    uint32_t totalSizeInframe = 0;
    uint32_t spanSizeInframe = 0;
    ret = audioEndpointInner->GetPreferBufferInfo(totalSizeInframe, spanSizeInframe);
    EXPECT_EQ(0, ret);
    audioEndpointInner->ProcessUpdateAppsUidForPlayback();
    uint32_t res = 0;
    res = audioEndpointInner->GetLinkedProcessCount();
    EXPECT_EQ(0, res);
}

/*
 * @tc.name  : Test CheckAudioHapticsSync API
 * @tc.type  : FUNC
 * @tc.number: CheckAudioHapticsSync_001
 * @tc.desc  : Test CheckAudioHapticsSync interface
 */
HWTEST_F(AudioEndpointUnitTest, CheckAudioHapticsSync_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);

    int32_t inValidSyncId = -1;
    audioEndpointInner->audioHapticsSyncId_ = inValidSyncId;
    audioEndpointInner->CheckAudioHapticsSync();
    EXPECT_EQ(audioEndpointInner->audioHapticsSyncId_, inValidSyncId);

    int32_t validSyncId = 1;
    int32_t zeroSyncId = 0;
    audioEndpointInner->audioHapticsSyncId_ = validSyncId;
    audioEndpointInner->CheckAudioHapticsSync();
    EXPECT_EQ(audioEndpointInner->audioHapticsSyncId_, zeroSyncId);

    audioEndpointInner->audioHapticsSyncId_ = validSyncId;
    audioEndpointInner->fastRenderId_ = HDI_INVALID_ID;
    audioEndpointInner->CheckAudioHapticsSync();
    EXPECT_EQ(audioEndpointInner->audioHapticsSyncId_, zeroSyncId);
}


/*
 * @tc.name  : Test SetVolume API
 * @tc.type  : FUNC
 * @tc.number: SetVolume_001
 * @tc.desc  : Test SetVolume interface
 */
HWTEST_F(AudioEndpointUnitTest, SetVolume_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->endpointType = AudioEndpointInner::TYPE_VOIP_MMAP;
    audioEndpointInner->fastRenderId_ = 1;
    float volume = 0.5f;
    AudioStreamType streamType = AudioStreamType::STREAM_VOICE_CALL;
    int32_t result = audioEndpointInner->SetVolume(streamType, volume);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * @tc.name  : Test SetVolume API
 * @tc.type  : FUNC
 * @tc.number: SetVolume_002
 * @tc.desc  : Test SetVolume interface
 */
HWTEST_F(AudioEndpointUnitTest, SetVolume_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->endpointType = AudioEndpointInner::TYPE_VOIP_MMAP;
    audioEndpointInner->fastRenderId_ = 1;
    float volume = 0.5f;
    AudioStreamType streamType = AudioStreamType::STREAM_MEDIA;
    int32_t result = audioEndpointInner->SetVolume(streamType, volume);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * @tc.name  : Test CheckPlaySignal API
 * @tc.type  : FUNC
 * @tc.number: CheckPlaySignal_001
 * @tc.desc  : Test CheckPlaySignal interface
 */
HWTEST_F(AudioEndpointUnitTest, CheckPlaySignal_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->latencyMeasEnabled_ = false;
    uint8_t buffer[10] = {0};
    audioEndpointInner->CheckPlaySignal(buffer, 10);
    EXPECT_EQ(audioEndpointInner->detectedTime_, 0);
}

/*
 * @tc.name  : Test CheckPlaySignal API
 * @tc.type  : FUNC
 * @tc.number: CheckPlaySignal_002
 * @tc.desc  : Test CheckPlaySignal interface
 */
HWTEST_F(AudioEndpointUnitTest, CheckPlaySignal_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->latencyMeasEnabled_ = true;
    audioEndpointInner->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    audioEndpointInner->signalDetectAgent_->signalDetected_ = true;
    audioEndpointInner->detectedTime_ = 1000;
    uint8_t buffer[10] = {0};
    audioEndpointInner->CheckPlaySignal(buffer, 10);
    EXPECT_EQ(audioEndpointInner->detectedTime_, 1000);
}

/*
 * @tc.name  : Test CheckPlaySignal API
 * @tc.type  : FUNC
 * @tc.number: CheckPlaySignal_003
 * @tc.desc  : Test CheckPlaySignal interface
 */
HWTEST_F(AudioEndpointUnitTest, CheckPlaySignal_003, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->latencyMeasEnabled_ = true;
    audioEndpointInner->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    audioEndpointInner->signalDetectAgent_->signalDetected_ = true;
    audioEndpointInner->detectedTime_ = 0;
    uint8_t buffer[10] = {0};
    audioEndpointInner->CheckPlaySignal(buffer, 10);
    EXPECT_EQ(audioEndpointInner->detectedTime_, 0);
}

/*
 * @tc.name  : Test CheckRecordSignal API
 * @tc.type  : FUNC
 * @tc.number: CheckRecordSignal_001
 * @tc.desc  : Test CheckRecordSignal interface
 */
HWTEST_F(AudioEndpointUnitTest, CheckRecordSignal_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->latencyMeasEnabled_ = false;
    uint8_t buffer[10] = {0};
    audioEndpointInner->CheckRecordSignal(buffer, 10);
}

/*
 * @tc.name  : Test CheckRecordSignal API
 * @tc.type  : FUNC
 * @tc.number: CheckRecordSignal_002
 * @tc.desc  : Test CheckRecordSignal interface
 */
HWTEST_F(AudioEndpointUnitTest, CheckRecordSignal_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->latencyMeasEnabled_ = true;
    audioEndpointInner->signalDetectAgent_ = nullptr;
    uint8_t buffer[10] = {0};
    audioEndpointInner->CheckRecordSignal(buffer, 10);
}

/*
 * @tc.name  : Test CheckRecordSignal API
 * @tc.type  : FUNC
 * @tc.number: CheckRecordSignal_003
 * @tc.desc  : Test CheckRecordSignal interface
 */
HWTEST_F(AudioEndpointUnitTest, CheckRecordSignal_003, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->latencyMeasEnabled_ = true;
    audioEndpointInner->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    audioEndpointInner->fastCaptureId_ = 1;
    uint8_t buffer[10] = {0};
    audioEndpointInner->CheckRecordSignal(buffer, 10);
}

/*
 * @tc.name  : Test ZeroVolumeCheck API
 * @tc.type  : FUNC
 * @tc.number: ZeroVolumeCheck_001
 * @tc.desc  : Test ZeroVolumeCheck interface
 */
HWTEST_F(AudioEndpointUnitTest, ZeroVolumeCheck_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->fastSinkType_ = AudioEndpointInner::FAST_SINK_TYPE_BLUETOOTH;
    audioEndpointInner->ZeroVolumeCheck(0);
    EXPECT_EQ(audioEndpointInner->zeroVolumeState_, AudioEndpointInner::INACTIVE);
}

/*
 * @tc.name  : Test ZeroVolumeCheck API
 * @tc.type  : FUNC
 * @tc.number: ZeroVolumeCheck_002
 * @tc.desc  : Test ZeroVolumeCheck interface
 */
HWTEST_F(AudioEndpointUnitTest, ZeroVolumeCheck_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->fastSinkType_ = AudioEndpointInner::FAST_SINK_TYPE_REMOTE;
    audioEndpointInner->ZeroVolumeCheck(0);
    EXPECT_EQ(audioEndpointInner->zeroVolumeState_, AudioEndpointInner::IN_TIMING);
}

/*
 * @tc.name  : Test ZeroVolumeCheck API
 * @tc.type  : FUNC
 * @tc.number: ZeroVolumeCheck_003
 * @tc.desc  : Test ZeroVolumeCheck interface
 */
HWTEST_F(AudioEndpointUnitTest, ZeroVolumeCheck_003, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->fastSinkType_ = AudioEndpointInner::FAST_SINK_TYPE_REMOTE;
    audioEndpointInner->zeroVolumeState_ = AudioEndpointInner::IN_TIMING;
    audioEndpointInner->zeroVolumeStartTime_ = ClockTime::GetCurNano() - 4000000000 - 1;
    audioEndpointInner->ZeroVolumeCheck(0);
    EXPECT_EQ(audioEndpointInner->zeroVolumeState_, AudioEndpointInner::ACTIVE);
}

/*
 * @tc.name  : Test ZeroVolumeCheck API
 * @tc.type  : FUNC
 * @tc.number: ZeroVolumeCheck_004
 * @tc.desc  : Test ZeroVolumeCheck interface
 */
HWTEST_F(AudioEndpointUnitTest, ZeroVolumeCheck_004, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->fastSinkType_ = AudioEndpointInner::FAST_SINK_TYPE_REMOTE;
    audioEndpointInner->ZeroVolumeCheck(1);
    EXPECT_EQ(audioEndpointInner->zeroVolumeState_, AudioEndpointInner::INACTIVE);
}

/*
 * @tc.name  : Test ZeroVolumeCheck API
 * @tc.type  : FUNC
 * @tc.number: ZeroVolumeCheck_005
 * @tc.desc  : Test ZeroVolumeCheck interface
 */
HWTEST_F(AudioEndpointUnitTest, ZeroVolumeCheck_005, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->fastSinkType_ = AudioEndpointInner::FAST_SINK_TYPE_REMOTE;
    audioEndpointInner->zeroVolumeState_ = AudioEndpointInner::ACTIVE;
    audioEndpointInner->ZeroVolumeCheck(1);
    EXPECT_EQ(audioEndpointInner->zeroVolumeState_, 0);
}

/*
 * @tc.name  : Test HandleZeroVolumeStartEvent API
 * @tc.type  : FUNC
 * @tc.number: HandleZeroVolumeStartEvent_001
 * @tc.desc  : Test HandleZeroVolumeStartEvent interface
 */
HWTEST_F(AudioEndpointUnitTest, HandleZeroVolumeStartEvent_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->isStarted_ = false;
    audioEndpointInner->HandleZeroVolumeStartEvent();
    EXPECT_FALSE(audioEndpointInner->isStarted_);
}

/*
 * @tc.name  : Test HandleZeroVolumeStartEvent API
 * @tc.type  : FUNC
 * @tc.number: HandleZeroVolumeStartEvent_002
 * @tc.desc  : Test HandleZeroVolumeStartEvent interface
 */
HWTEST_F(AudioEndpointUnitTest, HandleZeroVolumeStartEvent_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->isStarted_ = true;
    audioEndpointInner->HandleZeroVolumeStartEvent();
    EXPECT_TRUE(audioEndpointInner->isStarted_);
}

/*
 * @tc.name  : Test HandleZeroVolumeStopEvent API
 * @tc.type  : FUNC
 * @tc.number: HandleZeroVolumeStopEvent_001
 * @tc.desc  : Test HandleZeroVolumeStopEvent interface
 */
HWTEST_F(AudioEndpointUnitTest, HandleZeroVolumeStopEvent_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->isStarted_ = false;
    audioEndpointInner->HandleZeroVolumeStopEvent();
    EXPECT_FALSE(audioEndpointInner->isStarted_);
}

/*
 * @tc.name  : Test CheckStandBy API
 * @tc.type  : FUNC
 * @tc.number: CheckStandBy_001
 * @tc.desc  : Test CheckStandBy interface
 */
HWTEST_F(AudioEndpointUnitTest, CheckStandBy_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->endpointStatus_ = AudioEndpointInner::RUNNING;
    audioEndpointInner->clientConfig_.audioMode = AUDIO_MODE_PLAYBACK;
    audioEndpointInner->CheckStandBy();
    EXPECT_EQ(audioEndpointInner->endpointStatus_, AudioEndpointInner::RUNNING);
}

/*
 * @tc.name  : Test CheckStandBy API
 * @tc.type  : FUNC
 * @tc.number: CheckStandBy_002
 * @tc.desc  : Test CheckStandBy interface
 */
HWTEST_F(AudioEndpointUnitTest, CheckStandBy_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->endpointStatus_ = AudioEndpointInner::IDEL;
    audioEndpointInner->clientConfig_.audioMode = AUDIO_MODE_PLAYBACK;
    audioEndpointInner->CheckStandBy();
    EXPECT_EQ(audioEndpointInner->endpointStatus_, AudioEndpointInner::IDEL);
}

/*
 * @tc.name  : Test InitLatencyMeasurement API
 * @tc.type  : FUNC
 * @tc.number: InitLatencyMeasurement_001
 * @tc.desc  : Test InitLatencyMeasurement interface
 */
HWTEST_F(AudioEndpointUnitTest, InitLatencyMeasurement_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->InitLatencyMeasurement();
    EXPECT_FALSE(audioEndpointInner->latencyMeasEnabled_);
}

/*
 * @tc.name  : Test streamIndex API
 * @tc.type  : FUNC
 * @tc.number: streamIndex_001
 * @tc.desc  : Test streamIndex interface
 */
HWTEST_F(AudioEndpointUnitTest, streamIndex_001, TestSize.Level1)
{
    MockCallbacks mockCallbacks0(0);
    EXPECT_EQ(mockCallbacks0.streamIndex_, 0);

    MockCallbacks mockCallbacks1(1);
    EXPECT_EQ(mockCallbacks0.streamIndex_, 1);

    MockCallbacks mockCallbacks100(100);
    EXPECT_EQ(mockCallbacks0.streamIndex_, 100);
}

/*
 * @tc.name  : Test streamIndex API
 * @tc.type  : FUNC
 * @tc.number: streamIndex_002
 * @tc.desc  : Test streamIndex interface
 */
HWTEST_F(AudioEndpointUnitTest, streamIndex_002, TestSize.Level1)
{
    MockCallbacks mockCallbacks0(0);
    EXPECT_EQ(mockCallbacks0.streamIndex_, 0);
    EXPECT_EQ(mockCallbacks0.dumpDupOutFileName_, "0_endpoint_dup_out_.pcm");
    EXPECT_EQ(mockCallbacks0.dumpDupOut_, nullptr);

    MockCallbacks mockCallbacks1(1);
    EXPECT_EQ(mockCallbacks1.streamIndex_, 1);
    EXPECT_EQ(mockCallbacks1.dumpDupOutFileName_, "1_endpoint_dup_out_.pcm");
    EXPECT_EQ(mockCallbacks1.dumpDupOut_, nullptr);


    MockCallbacks mockCallbacks100(100);
    EXPECT_EQ(mockCallbacks100.streamIndex_, 100);
    EXPECT_EQ(mockCallbacks100.dumpDupOutFileName_, "100_endpoint_dup_out_.pcm");
    EXPECT_EQ(mockCallbacks100.dumpDupOut_, nullptr);
}

/*
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: OnWriteData_001
 * @tc.desc  : Test OnWriteData interface
 */
HWTEST_F(AudioEndpointUnitTest, OnWriteData_001, TestSize.Level1)
{
    MockCallbacks mockCallbacks0(0);
    size_t length = 10;
    int32_t result = mockCallbacks0.OnWriteData(length);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: OnWriteData_002
 * @tc.desc  : Test OnWriteData interface
 */
HWTEST_F(AudioEndpointUnitTest, OnWriteData_002, TestSize.Level1)
{
    MockCallbacks mockCallbacks0(0);
    size_t length = 0;
    int32_t result = mockCallbacks0.OnWriteData(length);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: OnWriteData_003
 * @tc.desc  : Test OnWriteData interface
 */
HWTEST_F(AudioEndpointUnitTest, OnWriteData_003, TestSize.Level1)
{
    MockCallbacks mockCallbacks0(0);
    size_t length = std::numeric_limits<size_t>::max();
    int32_t result = mockCallbacks0.OnWriteData(length);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: OnWriteData_004
 * @tc.desc  : Test OnWriteData interface
 */
HWTEST_F(AudioEndpointUnitTest, OnWriteData_004, TestSize.Level1)
{
    MockCallbacks mockCallbacks0(0);
    int8_t inputData[10] = {0};
    size_t requestDataLen = 10;
    int32_t result = mockCallbacks0.OnWriteData(inputData, requestDataLen);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test EnableFastInnerCap API
 * @tc.type  : FUNC
 * @tc.number: AudioEnableFastInnerCap_003
 * @tc.desc  : Test EnableFastInnerCap interface.
 */
HWTEST_F(AudioEndpointUnitTest, AudioEnableFastInnerCap_003, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    DeviceStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, CH_LAYOUT_STEREO };
    deviceInfo.audioStreamInfo_ = { audioStreamInfo };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    auto &info = audioEndpointInner->fastCaptureInfos_[1];
    info.isInnerCapEnabled = true;
    audioEndpointInner->deviceInfo_.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    int32_t result = audioEndpointInner->EnableFastInnerCap(1);
    EXPECT_EQ(SUCCESS, result);

    audioEndpointInner->endpointStatus_ = AudioEndpointInner::RUNNING;
    EXPECT_EQ(SUCCESS, audioEndpointInner->EnableFastInnerCap(1));

    audioEndpointInner->endpointStatus_.store(AudioEndpoint::EndpointStatus::IDEL);
    audioEndpointInner->isDeviceRunningInIdel_ = true;
    EXPECT_EQ(SUCCESS, audioEndpointInner->EnableFastInnerCap(1));
}

/*
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: Dump_001
 * @tc.desc  : Test Dump interface
 */
HWTEST_F(AudioEndpointUnitTest, Dump_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_MMAP);
    EXPECT_NE(nullptr, audioEndpointInner);

    std::string dumpString = "";
    audioEndpointInner->dstAudioBuffer_  = nullptr;
    audioEndpointInner->Dump(dumpString);
    ASSERT_STRNE("", dumpString.c_str());
}

/*
 * @tc.name  : Test ZeroVolumeCheck API
 * @tc.type  : FUNC
 * @tc.number: ZeroVolumeCheck_006
 * @tc.desc  : Test ZeroVolumeCheck interface
 */
HWTEST_F(AudioEndpointUnitTest, ZeroVolumeCheck_006, TestSize.Level1)
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    DeviceStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, CH_LAYOUT_STEREO };
    deviceInfo.audioStreamInfo_ = { audioStreamInfo };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->zeroVolumeState_ = AudioEndpointInner::IN_TIMING;
    audioEndpointInner->ZeroVolumeCheck(0);
    EXPECT_EQ(AudioEndpointInner::IN_TIMING, audioEndpointInner->zeroVolumeState_);

    usleep(4000000); // 2000000 for sleep 2s, wait for 4s limitation
    audioEndpointInner->zeroVolumeState_ = AudioEndpointInner::IN_TIMING;
    audioEndpointInner->isStarted_ = true;
    audioEndpointInner->ZeroVolumeCheck(0); //enter check and stop device
    EXPECT_EQ(AudioEndpointInner::IN_TIMING, audioEndpointInner->zeroVolumeState_);

    audioEndpointInner->zeroVolumeState_ = AudioEndpointInner::IN_TIMING;
    audioEndpointInner->isStarted_ = true;
    audioEndpointInner->ZeroVolumeCheck(1); //enter check and start device
    EXPECT_EQ(AudioEndpointInner::INACTIVE, audioEndpointInner->zeroVolumeState_);

    audioEndpointInner->zeroVolumeState_ = AudioEndpointInner::IN_TIMING;
    audioEndpointInner->ZeroVolumeCheck(1);
    EXPECT_EQ(AudioEndpointInner::INACTIVE, audioEndpointInner->zeroVolumeState_);

    HdiAdapterManager::GetInstance().ReleaseId(audioEndpointInner->fastRenderId_);
}

/*
 * @tc.name  : Test HandleZeroVolumeStartEvent API
 * @tc.type  : FUNC
 * @tc.number: TestHandleZeroVolumeStartEvent_001
 * @tc.desc  : Test HandleZeroVolumeStartEvent interface
 */
HWTEST(AudioEndpointInnerUnitTest, TestHandleZeroVolumeStartEvent_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->isStarted_ = true;
    audioEndpointInner->HandleZeroVolumeStartEvent();
    EXPECT_EQ(true, audioEndpointInner->isStarted_);
    EXPECT_EQ(true, audioEndpointInner->needReSyncPosition_);
}

/*
 * @tc.name  : Test HandleZeroVolumeStartEvent API
 * @tc.type  : FUNC
 * @tc.number: TestHandleZeroVolumeStartEvent_002
 * @tc.desc  : Test HandleZeroVolumeStartEvent interface
 */
HWTEST(AudioEndpointInnerUnitTest, TestHandleZeroVolumeStartEvent_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->isStarted_ = false;
    audioEndpointInner->HandleZeroVolumeStartEvent();
    EXPECT_EQ(false, audioEndpointInner->isStarted_);
    EXPECT_EQ(true, audioEndpointInner->needReSyncPosition_);
}

/*
 * @tc.name  : Test CheckRecordSignal API
 * @tc.type  : FUNC
 * @tc.number: CheckRecordSignal_004
 * @tc.desc  : Test CheckRecordSignal interface
 */
HWTEST(AudioEndpointInnerUnitTest, CheckRecordSignal_004, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->latencyMeasEnabled_ = true;
    audioEndpointInner->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    audioEndpointInner->fastCaptureId_ = 1;
    audioEndpointInner->signalDetected_ = true;
    uint8_t buffer[10] = {0};
    audioEndpointInner->CheckRecordSignal(buffer, 10);
    EXPECT_NE(nullptr, audioEndpointInner->signalDetectAgent_);
}

/*
 * @tc.name  : Test CheckPlaySignal API
 * @tc.type  : FUNC
 * @tc.number: CheckPlaySignal_004
 * @tc.desc  : Test CheckPlaySignal interface
 */
HWTEST_F(AudioEndpointUnitTest, CheckPlaySignal_004, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->latencyMeasEnabled_ = true;
    audioEndpointInner->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    audioEndpointInner->detectedTime_ = 0;
    audioEndpointInner->signalDetected_ = true;
    uint8_t buffer[10] = {0};

    audioEndpointInner->CheckPlaySignal(buffer, 10);
    EXPECT_EQ(false, audioEndpointInner->signalDetectAgent_->dspTimestampGot_);

    audioEndpointInner->signalDetectAgent_->signalDetected_ = true;
    audioEndpointInner->signalDetectAgent_->dspTimestampGot_ = true;
    audioEndpointInner->CheckPlaySignal(buffer, 10);
    EXPECT_EQ(false, audioEndpointInner->detectedTime_);

    audioEndpointInner->detectedTime_ = 1000;
    audioEndpointInner->CheckPlaySignal(buffer, 10);
    EXPECT_EQ(1000, audioEndpointInner->detectedTime_);
}
} // namespace AudioStandard
} // namespace OHOS
