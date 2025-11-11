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
#include "audio_endpoint_sink_adapter.cpp"

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
        std::make_shared<AudioEndpointInner>(type, id, clientConfig.audioMode);
    CHECK_AND_RETURN_RET_LOG(audioEndpoint != nullptr, nullptr, "Create AudioEndpoint failed.");

    if (!audioEndpoint->Config(deviceInfo, streamInfo, clientConfig.streamType)) {
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
        std::make_shared<AudioEndpointInner>(type, AUDIO_ENDPOINT_ID, config.audioMode);
    audioEndpoint->injector_ = *MockAudioInjector::GetMockInstance();
    if (!audioEndpoint->Config(deviceInfo, audioStreamInfo, config.streamType)) {
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

    int32_t ret = audioEndpointInner->LinkProcessStream(processStream);
    EXPECT_EQ(SUCCESS, ret);

    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    result = audioEndpointInner->Config(deviceInfo, audioStreamInfo, config.streamType);
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
    EXPECT_FALSE(audioEndpointInner->signalDetected_);
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
    EXPECT_FALSE(audioEndpointInner->signalDetected_);
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
    audioEndpointInner->audioMode_ = AUDIO_MODE_PLAYBACK;
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
    audioEndpointInner->audioMode_ = AUDIO_MODE_PLAYBACK;
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

/*
 * @tc.name  : Test IsDupRenderCallbackMode API
 * @tc.type  : FUNC
 * @tc.number: IsDupRenderCallbackMode_001
 * @tc.desc  : Test IsDupRenderCallbackMode interface
 */
HWTEST(AudioEndpointInnerUnitTest, IsDupRenderCallbackMode_001, TestSize.Level1)
{
    EXPECT_EQ(AudioEndpointInner::IsDupRenderCallbackMode(1, true), false);
    EXPECT_EQ(AudioEndpointInner::IsDupRenderCallbackMode(1, false), true);
    EXPECT_EQ(AudioEndpointInner::IsDupRenderCallbackMode(0, true), false);
    EXPECT_EQ(AudioEndpointInner::IsDupRenderCallbackMode(0, false), false);
}

/*
 * @tc.name  : Test IsDupRenderCallbackMode API
 * @tc.type  : FUNC
 * @tc.number: IsDualStream_001
 * @tc.desc  : Test IsDualStream interface
 */
HWTEST(AudioEndpointInnerUnitTest, IsDualStream_001, TestSize.Level1)
{
    CaptureInfo capInfo = {
        .dualDeviceName = "Speaker"
    };
    EXPECT_EQ(AudioEndpointInner::IsDualStream(capInfo), true);
}

/*
 * @tc.name  : Test IsDupRenderCallbackMode API
 * @tc.type  : FUNC
 * @tc.number: IsDualStream_002
 * @tc.desc  : Test IsDualStream interface
 */
HWTEST(AudioEndpointInnerUnitTest, IsDualStream_002, TestSize.Level1)
{
    CaptureInfo capInfo;
    EXPECT_EQ(AudioEndpointInner::IsDualStream(capInfo), false);
}

/**
 * @tc.name  : Test AddCaptureInjector API
 * @tc.type  : FUNC
 * @tc.number: AddCaptureInjector_001
 * @tc.desc  : Test AddCaptureInjector with valid VOICE_COMMUNICATION source type and matching sink port index.
 */
HWTEST(AudioEndpointInnerUnitTest, AddCaptureInjector_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);

    // Set up required member variables
    audioEndpointInner->dstStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    audioEndpointInner->dstStreamInfo_.format = SAMPLE_S16LE;
    audioEndpointInner->dstStreamInfo_.channels = STEREO;
    audioEndpointInner->injector_.sinkPortIndex_ = 1234;

    uint32_t sinkPortIndex = 1234;
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;

    int32_t result = audioEndpointInner->AddCaptureInjector(sinkPortIndex, sourceType);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_TRUE(audioEndpointInner->isNeedInject_);
    EXPECT_EQ(audioEndpointInner->injectSinkPortIdx_, sinkPortIndex);
}

/**
 * @tc.name  : Test AddCaptureInjector API
 * @tc.type  : FUNC
 * @tc.number: AddCaptureInjector_002
 * @tc.desc  : Test AddCaptureInjector with invalid source type, should return ERROR.
 */
HWTEST(AudioEndpointInnerUnitTest, AddCaptureInjector_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);

    uint32_t sinkPortIndex = 1234;
    SourceType sourceType = SOURCE_TYPE_MIC; // Invalid source type

    int32_t result = audioEndpointInner->AddCaptureInjector(sinkPortIndex, sourceType);

    EXPECT_EQ(result, ERROR);
    EXPECT_FALSE(audioEndpointInner->isNeedInject_);
    EXPECT_NE(audioEndpointInner->injectSinkPortIdx_, sinkPortIndex);
}

/**
 * @tc.name  : Test AddCaptureInjector API
 * @tc.type  : FUNC
 * @tc.number: AddCaptureInjector_003
 * @tc.desc  : Test AddCaptureInjector with mismatched sink port index, should return ERROR.
 */
HWTEST(AudioEndpointInnerUnitTest, AddCaptureInjector_003, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);

    // Mock injector to return different port index
    EXPECT_CALL(*MockAudioInjector::GetMockInstance(), GetSinkPortIdx())
        .WillOnce(Return(5678)); // Different from input

    uint32_t sinkPortIndex = 1234;
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    int32_t result = audioEndpointInner->AddCaptureInjector(sinkPortIndex, sourceType);

    EXPECT_EQ(result, ERROR);
    EXPECT_FALSE(audioEndpointInner->isNeedInject_);
    EXPECT_NE(audioEndpointInner->injectSinkPortIdx_, sinkPortIndex);
}

/**
 * @tc.name  : Test RemoveCaptureInjector API
 * @tc.type  : FUNC
 * @tc.number: RemoveCaptureInjector_001
 * @tc.desc  : Test RemoveCaptureInjector with valid VOICE_COMMUNICATION source type and matching sink port index.
 */
HWTEST(AudioEndpointInnerUnitTest, RemoveCaptureInjector_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);

    // Set up initial state
    audioEndpointInner->isNeedInject_ = true;
    audioEndpointInner->injectSinkPortIdx_ = 1234;

    uint32_t sinkPortIndex = 1234;
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;

    int32_t result = audioEndpointInner->RemoveCaptureInjector(sinkPortIndex, sourceType);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_FALSE(audioEndpointInner->isNeedInject_);
    EXPECT_EQ(audioEndpointInner->injectSinkPortIdx_, UINT32_INVALID_VALUE);
}

/**
 * @tc.name  : Test RemoveCaptureInjector API
 * @tc.type  : FUNC
 * @tc.number: RemoveCaptureInjector_002
 * @tc.desc  : Test RemoveCaptureInjector with invalid source type, should return ERROR.
 */
HWTEST(AudioEndpointInnerUnitTest, RemoveCaptureInjector_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);

    // Set up initial state
    audioEndpointInner->isNeedInject_ = true;
    audioEndpointInner->injectSinkPortIdx_ = 1234;

    uint32_t sinkPortIndex = 1234;
    SourceType sourceType = SOURCE_TYPE_MIC; // Invalid source type

    int32_t result = audioEndpointInner->RemoveCaptureInjector(sinkPortIndex, sourceType);

    EXPECT_EQ(result, ERROR);
    EXPECT_TRUE(audioEndpointInner->isNeedInject_); // Should remain unchanged
    EXPECT_EQ(audioEndpointInner->injectSinkPortIdx_, 1234); // Should remain unchanged
}

/**
 * @tc.name  : Test RemoveCaptureInjector API
 * @tc.type  : FUNC
 * @tc.number: RemoveCaptureInjector_003
 * @tc.desc  : Test RemoveCaptureInjector with mismatched sink port index, should return ERROR.
 */
HWTEST(AudioEndpointInnerUnitTest, RemoveCaptureInjector_003, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);

    // Set up initial state
    audioEndpointInner->isNeedInject_ = true;
    audioEndpointInner->injectSinkPortIdx_ = 1234;

    uint32_t sinkPortIndex = 5678; // Different from stored index
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    
    int32_t result = audioEndpointInner->RemoveCaptureInjector(sinkPortIndex, sourceType);

    EXPECT_EQ(result, ERROR);
    EXPECT_TRUE(audioEndpointInner->isNeedInject_); // Should remain unchanged
    EXPECT_EQ(audioEndpointInner->injectSinkPortIdx_, 1234); // Should remain unchanged
}

/**
 * @tc.name  : Test AddRemoveCaptureInjector API sequence
 * @tc.type  : FUNC
 * @tc.number: AddRemoveCaptureInjector_001
 * @tc.desc  : Test sequential Add and Remove operations with valid parameters.
 */
HWTEST(AudioEndpointInnerUnitTest, AddRemoveCaptureInjector_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_MMAP);

    // Set up required member variables
    audioEndpointInner->dstStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    audioEndpointInner->dstStreamInfo_.format = SAMPLE_S16LE;
    audioEndpointInner->dstStreamInfo_.channels = STEREO;
    audioEndpointInner->injector_.sinkPortIndex_ = 1234;

    uint32_t sinkPortIndex = 1234;
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;

    // Test multiple add/remove cycles
    for (int i = 0; i < 3; i++) {
        // Add injector
        int32_t addResult = audioEndpointInner->AddCaptureInjector(sinkPortIndex, sourceType);
        EXPECT_EQ(addResult, SUCCESS);
        EXPECT_TRUE(audioEndpointInner->isNeedInject_);
        EXPECT_EQ(audioEndpointInner->injectSinkPortIdx_, sinkPortIndex);
        
        // Remove injector
        int32_t removeResult = audioEndpointInner->RemoveCaptureInjector(sinkPortIndex, sourceType);
        EXPECT_EQ(removeResult, SUCCESS);
        EXPECT_FALSE(audioEndpointInner->isNeedInject_);
        EXPECT_EQ(audioEndpointInner->injectSinkPortIdx_, UINT32_INVALID_VALUE);
    }
}

/**
 * @tc.name  : Test InjectToCaptureDataProc API
 * @tc.type  : FUNC
 * @tc.number: InjectToCaptureDataProc_001
 * @tc.desc  : Test InjectToCaptureDataProc with injection not needed, should return early.
 */
HWTEST(AudioEndpointInnerUnitTest, InjectToCaptureDataProc_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);

    SetInjectEnable(true);
    audioEndpointInner->isNeedInject_ = false;

    BufferDesc readBuf = {nullptr, 1024};
    audioEndpointInner->InjectToCaptureDataProc(readBuf);

    // Should return early without processing
    EXPECT_FALSE(audioEndpointInner->isConvertReadFormat_);
}

/**
 * @tc.name  : Test InjectToCaptureDataProc API
 * @tc.type  : FUNC
 * @tc.number: InjectToCaptureDataProc_002
 * @tc.desc  : Test InjectToCaptureDataProc with wrong endpoint type, should return early.
 */
HWTEST(AudioEndpointInnerUnitTest, InjectToCaptureDataProc_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_MMAP);

    SetInjectEnable(true);
    audioEndpointInner->isNeedInject_ = true;
    audioEndpointInner->endpointType_ = AudioEndpoint::TYPE_MMAP; // Not VOIP_MMAP

    BufferDesc readBuf = {nullptr, 1024};
    audioEndpointInner->InjectToCaptureDataProc(readBuf);

    // Should return early without processing
    EXPECT_FALSE(audioEndpointInner->isConvertReadFormat_);
}

/**
 * @tc.name  : Test InjectToCaptureDataProc API
 * @tc.type  : FUNC
 * @tc.number: InjectToCaptureDataProc_003
 * @tc.desc  : Test InjectToCaptureDataProc with successful injection processing.
 */
HWTEST(AudioEndpointInnerUnitTest, InjectToCaptureDataProc_003, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);

    // Set up required state
    audioEndpointInner->isNeedInject_ = true;
    audioEndpointInner->endpointType_ = AudioEndpoint::TYPE_VOIP_MMAP;
    audioEndpointInner->dstStreamInfo_.channels = STEREO;
    audioEndpointInner->dstStreamInfo_.format = SAMPLE_S16LE;
    audioEndpointInner->dstStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    audioEndpointInner->injectSinkPortIdx_ = 1234;

    SetInjectEnable(true);

    // Create test buffer
    BufferDesc readBuf = {nullptr, 1024};
    audioEndpointInner->InjectToCaptureDataProc(readBuf);

    // Should complete processing successfully
    EXPECT_TRUE(audioEndpointInner->isConvertReadFormat_);
}

/**
 * @tc.name  : Test InjectToCaptureDataProc API
 * @tc.type  : FUNC
 * @tc.number: InjectToCaptureDataProc_004
 * @tc.desc  : Test InjectToCaptureDataProc with PeekRendererInjectData failure.
 */
HWTEST(AudioEndpointInnerUnitTest, InjectToCaptureDataProc_004, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);

    // Set up required state
    audioEndpointInner->isNeedInject_ = true;
    audioEndpointInner->endpointType_ = AudioEndpoint::TYPE_VOIP_MMAP;
    audioEndpointInner->dstStreamInfo_.channels = STEREO;
    audioEndpointInner->dstStreamInfo_.format = SAMPLE_S16LE;
    audioEndpointInner->dstStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    audioEndpointInner->injectSinkPortIdx_ = 1234;
    audioEndpointInner->fastCaptureId_ = 1;

    SetInjectEnable(true);

    BufferDesc readBuf = {nullptr, 1024};
    audioEndpointInner->InjectToCaptureDataProc(readBuf);

    // Should return early due to peek failure
    EXPECT_FALSE(audioEndpointInner->isConvertReadFormat_);
}

/**
 * @tc.name  : Test InjectToCaptureDataProc API
 * @tc.type  : FUNC
 * @tc.number: InjectToCaptureDataProc_005
 * @tc.desc  : Test InjectToCaptureDataProc with ConvertDataFormat failure.
 */
HWTEST(AudioEndpointInnerUnitTest, InjectToCaptureDataProc_005, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);

    // Set up required state
    audioEndpointInner->isNeedInject_ = true;
    audioEndpointInner->endpointType_ = AudioEndpoint::TYPE_VOIP_MMAP;
    audioEndpointInner->dstStreamInfo_.channels = STEREO; // Different from renderer
    audioEndpointInner->dstStreamInfo_.format = SAMPLE_S16LE;
    audioEndpointInner->dstStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    audioEndpointInner->injectSinkPortIdx_ = 1234;
    audioEndpointInner->fastCaptureId_ = 1;
    audioEndpointInner->limiter_ = std::make_shared<AudioLimiter>(1);
    audioEndpointInner->limiter_->algoFrameLen_ = 1;
    SetInjectEnable(true);

    BufferDesc readBuf = {nullptr, 1024};
    audioEndpointInner->InjectToCaptureDataProc(readBuf);

    // Should return early due to format conversion failure (channel mismatch)
    EXPECT_FALSE(audioEndpointInner->isConvertReadFormat_);
}

/**
 * @tc.name  : Test IsOtherEndpointRunning API when no other endpoints are running
 * @tc.type  : FUNC
 * @tc.number: IsOtherEndpointRunning_001
 * @tc.desc  : Test IsOtherEndpointRunning when no other endpoints are present
 */
HWTEST_F(AudioEndpointUnitTest, IsOtherEndpointRunning_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointSinkAdapter> checker = AudioEndpointSinkAdapter::GetInstance();
    uint32_t fastRenderId = 123;
    AudioEndpointSinkAdapter::EndpointName  key = "test_key";
    EXPECT_FALSE(checker->IsOtherEndpointRunning(fastRenderId, key));
}

/**
 * @tc.name  : Test IsOtherEndpointRunning API when other endpoints is IDEL
 * @tc.type  : FUNC
 * @tc.number: IsOtherEndpointRunning_002
 * @tc.desc  : Test IsOtherEndpointRunning when another endpoints is IDEL
 */
HWTEST_F(AudioEndpointUnitTest, IsOtherEndpointRunning_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointSinkAdapter> checker = AudioEndpointSinkAdapter::GetInstance();
    uint32_t fastRenderId = 123;
    AudioEndpointSinkAdapter::EndpointName  key = "test_key";
    AudioEndpointSinkAdapter::EndpointName  otherKey = "other_key";

    checker->AddOperation(fastRenderId, otherKey, AudioEndpoint::EndpointStatus::IDEL);
    EXPECT_FALSE(checker->IsOtherEndpointRunning(fastRenderId, key));
}

/**
 * @tc.name  : Test IsOtherEndpointRunning API when other endpoints is UNLINKED
 * @tc.type  : FUNC
 * @tc.number: IsOtherEndpointRunning_003
 * @tc.desc  : Test IsOtherEndpointRunning when another endpoints is UNLINKED
 */
HWTEST_F(AudioEndpointUnitTest, IsOtherEndpointRunning_003, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointSinkAdapter> checker = AudioEndpointSinkAdapter::GetInstance();
    uint32_t fastRenderId = 123;
    AudioEndpointSinkAdapter::EndpointName  key = "test_key";
    AudioEndpointSinkAdapter::EndpointName  otherKey = "other_key";

    checker->AddOperation(fastRenderId, otherKey, AudioEndpoint::EndpointStatus::UNLINKED);
    EXPECT_FALSE(checker->IsOtherEndpointRunning(fastRenderId, key));
}

/**
 * @tc.name  : Test IsOtherEndpointRunning API when other endpoints is RUNNING
 * @tc.type  : FUNC
 * @tc.number: IsOtherEndpointRunning_004
 * @tc.desc  : Test IsOtherEndpointRunning when another endpoints is RUNNING
 */
HWTEST_F(AudioEndpointUnitTest, IsOtherEndpointRunning_004, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointSinkAdapter> checker = AudioEndpointSinkAdapter::GetInstance();
    uint32_t fastRenderId = 123;
    AudioEndpointSinkAdapter::EndpointName  key = "test_key";
    AudioEndpointSinkAdapter::EndpointName  otherKey = "other_key";

    checker->AddOperation(fastRenderId, key, AudioEndpoint::EndpointStatus::IDEL);
    checker->AddOperation(fastRenderId, otherKey, AudioEndpoint::EndpointStatus::RUNNING);
    EXPECT_TRUE(checker->IsOtherEndpointRunning(fastRenderId, key));
}

/**
 * @tc.name  : Test UpdateStatus API
 * @tc.type  : FUNC
 * @tc.number: UpdateStatus_001
 * @tc.desc  : Test UpdateStatus to ensure it updates the status correctly
 */
HWTEST_F(AudioEndpointUnitTest, UpdateStatus_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointSinkAdapter> checker = AudioEndpointSinkAdapter::GetInstance();
    uint32_t fastRenderId = 123;
    AudioEndpointSinkAdapter::EndpointName  key = "test_key";
    AudioEndpoint::EndpointStatus initialStatus = AudioEndpoint::EndpointStatus::RUNNING;
    AudioEndpoint::EndpointStatus newStatus = AudioEndpoint::EndpointStatus::STOPPED;

    checker->AddOperation(fastRenderId, key, initialStatus);
    checker->UpdateStatus(fastRenderId, key, newStatus);

    std::lock_guard<std::mutex> lock(checker->checkerOperationMapMutex_);
    auto fastRenderIt = checker->operationMap.find(fastRenderId);
    EXPECT_NE(fastRenderIt, checker->operationMap.end());
    for (const auto &pair : fastRenderIt->second) {
        if (pair.first == key) {
            EXPECT_EQ(pair.second, newStatus);
            break;
        }
    }
}

/**
 * @tc.name  : Test UpdateStatus API for IDEL status
 * @tc.type  : FUNC
 * @tc.number: UpdateStatus_002
 * @tc.desc  : Test UpdateStatus to ensure it updates to IDEL status correctly
 */
HWTEST_F(AudioEndpointUnitTest, UpdateStatus_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointSinkAdapter> checker = AudioEndpointSinkAdapter::GetInstance();
    uint32_t fastRenderId = 123;
    AudioEndpointSinkAdapter::EndpointName  key = "test_key";
    AudioEndpoint::EndpointStatus initialStatus = AudioEndpoint::EndpointStatus::RUNNING;
    AudioEndpoint::EndpointStatus newStatus = AudioEndpoint::EndpointStatus::IDEL;

    checker->AddOperation(fastRenderId, key, initialStatus);
    checker->UpdateStatus(fastRenderId, key, newStatus);

    std::lock_guard<std::mutex> lock(checker->checkerOperationMapMutex_);
    auto fastRenderIt = checker->operationMap.find(fastRenderId);
    EXPECT_NE(fastRenderIt, checker->operationMap.end());
    for (const auto &pair : fastRenderIt->second) {
        if (pair.first == key) {
            EXPECT_EQ(pair.second, newStatus);
            break;
        }
    }
}

/**
 * @tc.name  : Test UpdateStatus API for STARTING status
 * @tc.type  : FUNC
 * @tc.number: UpdateStatus_003
 * @tc.desc  : Test UpdateStatus to ensure it updates to STARTING status correctly
 */
HWTEST_F(AudioEndpointUnitTest, UpdateStatus_003, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointSinkAdapter> checker = AudioEndpointSinkAdapter::GetInstance();
    uint32_t fastRenderId = 123;
    AudioEndpointSinkAdapter::EndpointName  key = "test_key";
    AudioEndpoint::EndpointStatus initialStatus = AudioEndpoint::EndpointStatus::RUNNING;
    AudioEndpoint::EndpointStatus newStatus = AudioEndpoint::EndpointStatus::STARTING;

    checker->AddOperation(fastRenderId, key, initialStatus);
    checker->UpdateStatus(fastRenderId, key, newStatus);

    std::lock_guard<std::mutex> lock(checker->checkerOperationMapMutex_);
    auto fastRenderIt = checker->operationMap.find(fastRenderId);
    EXPECT_NE(fastRenderIt, checker->operationMap.end());
    for (const auto &pair : fastRenderIt->second) {
        if (pair.first == key) {
            EXPECT_EQ(pair.second, newStatus);
            break;
        }
    }
}

/**
 * @tc.name  : Test UpdateStatus API for UNLINKED status
 * @tc.type  : FUNC
 * @tc.number: UpdateStatus_004
 * @tc.desc  : Test UpdateStatus to ensure it updates to UNLINKED status correctly
 */
HWTEST_F(AudioEndpointUnitTest, UpdateStatus_004, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointSinkAdapter> checker = AudioEndpointSinkAdapter::GetInstance();
    uint32_t fastRenderId = 123;
    AudioEndpointSinkAdapter::EndpointName  key = "test_key";
    AudioEndpoint::EndpointStatus initialStatus = AudioEndpoint::EndpointStatus::RUNNING;
    AudioEndpoint::EndpointStatus newStatus = AudioEndpoint::EndpointStatus::UNLINKED;

    checker->AddOperation(fastRenderId, key, initialStatus);
    checker->UpdateStatus(fastRenderId, key, newStatus);

    std::lock_guard<std::mutex> lock(checker->checkerOperationMapMutex_);
    auto fastRenderIt = checker->operationMap.find(fastRenderId);
    EXPECT_NE(fastRenderIt, checker->operationMap.end());
    for (const auto &pair : fastRenderIt->second) {
        if (pair.first == key) {
            EXPECT_EQ(pair.second, newStatus);
            break;
        }
    }
}

/**
 * @tc.name  : Test UpdateEndpointStatus API for RUNNING status
 * @tc.type  : FUNC
 * @tc.number: UpdateEndpointStatus_001
 * @tc.desc  : Test UpdateEndpointStatus to ensure it updates to RUNNING status correctly
 */
HWTEST_F(AudioEndpointUnitTest, UpdateEndpointStatus_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointSinkAdapter> checker = AudioEndpointSinkAdapter::GetInstance();
    uint32_t fastRenderId = 123;
    std::string endpointName = "test_endpoint";
    AudioEndpoint::EndpointStatus newStatus = AudioEndpoint::EndpointStatus::RUNNING;

    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;

    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->fastRenderId_ = fastRenderId;
    audioEndpointInner->UpdateEndpointStatus(newStatus);

    std::lock_guard<std::mutex> lock(checker->checkerOperationMapMutex_);
    auto fastRenderIt = checker->operationMap.find(fastRenderId);
    EXPECT_NE(fastRenderIt, checker->operationMap.end());
    for (const auto &pair : fastRenderIt->second) {
        if (pair.first == endpointName) {
            EXPECT_EQ(pair.second, newStatus);
            break;
        }
    }
}

/**
 * @tc.name  : Test UpdateEndpointStatus API for IDEL status
 * @tc.type  : FUNC
 * @tc.number: UpdateEndpointStatus_002
 * @tc.desc  : Test UpdateEndpointStatus to ensure it updates to IDEL status correctly
 */
HWTEST_F(AudioEndpointUnitTest, UpdateEndpointStatus_002, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointSinkAdapter> checker = AudioEndpointSinkAdapter::GetInstance();
    uint32_t fastRenderId = 123;
    std::string endpointName = "test_endpoint";
    AudioEndpoint::EndpointStatus newStatus = AudioEndpoint::EndpointStatus::IDEL;

    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;

    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->fastRenderId_ = fastRenderId;
    audioEndpointInner->UpdateEndpointStatus(newStatus);

    std::lock_guard<std::mutex> lock(checker->checkerOperationMapMutex_);
    auto fastRenderIt = checker->operationMap.find(fastRenderId);
    EXPECT_NE(fastRenderIt, checker->operationMap.end());
    for (const auto &pair : fastRenderIt->second) {
        if (pair.first == endpointName) {
            EXPECT_EQ(pair.second, newStatus);
            break;
        }
    }
}

/**
 * @tc.name  : Test UpdateEndpointStatus API for STARTING status
 * @tc.type  : FUNC
 * @tc.number: UpdateEndpointStatus_003
 * @tc.desc  : Test UpdateEndpointStatus to ensure it updates to STARTING status correctly
 */
HWTEST_F(AudioEndpointUnitTest, UpdateEndpointStatus_003, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointSinkAdapter> checker = AudioEndpointSinkAdapter::GetInstance();
    uint32_t fastRenderId = 123;
    std::string endpointName = "test_endpoint";
    AudioEndpoint::EndpointStatus newStatus = AudioEndpoint::EndpointStatus::STARTING;

    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;

    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->fastRenderId_ = fastRenderId;
    audioEndpointInner->UpdateEndpointStatus(newStatus);

    std::lock_guard<std::mutex> lock(checker->checkerOperationMapMutex_);
    auto fastRenderIt = checker->operationMap.find(fastRenderId);
    EXPECT_NE(fastRenderIt, checker->operationMap.end());
    for (const auto &pair : fastRenderIt->second) {
        if (pair.first == endpointName) {
            EXPECT_EQ(pair.second, newStatus);
            break;
        }
    }
}

/**
 * @tc.name  : Test UpdateEndpointStatus API for UNLINKED status
 * @tc.type  : FUNC
 * @tc.number: UpdateEndpointStatus_004
 * @tc.desc  : Test UpdateEndpointStatus to ensure it updates to UNLINKED status correctly
 */
HWTEST_F(AudioEndpointUnitTest, UpdateEndpointStatus_004, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointSinkAdapter> checker = AudioEndpointSinkAdapter::GetInstance();
    uint32_t fastRenderId = 123;
    std::string endpointName = "test_endpoint";
    AudioEndpoint::EndpointStatus newStatus = AudioEndpoint::EndpointStatus::UNLINKED;

    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;

    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo, audioStreamInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->fastRenderId_ = fastRenderId;
    audioEndpointInner->UpdateEndpointStatus(newStatus);

    std::lock_guard<std::mutex> lock(checker->checkerOperationMapMutex_);
    auto fastRenderIt = checker->operationMap.find(fastRenderId);
    EXPECT_NE(fastRenderIt, checker->operationMap.end());
    for (const auto &pair : fastRenderIt->second) {
        if (pair.first == endpointName) {
            EXPECT_EQ(pair.second, newStatus);
            break;
        }
    }
}

/**
 * @ tc.name : Test MixRendererAndCaptureData Function
 * @ tc.type : FUNC
 * @ tc.number: MixRendererAndCaptureData_001
 * @ tc.desc : Test MixRendererAndCaptureData function with basic input data.
 */
HWTEST(AudioEndpointInnerUnitTest, MixRendererAndCaptureData_001, TestSize.Level1)
{
    const size_t bufLength = sizeof(float); // 1 float
    std::vector<uint8_t> rendererConvData(bufLength, 0);
    BufferDesc rendererConvDesc;
    rendererConvDesc.bufLength = bufLength;
    rendererConvDesc.buffer = rendererConvData.data();
    float* leftBuff = reinterpret_cast<float*>(rendererConvDesc.buffer);
    leftBuff[0] = 0.5f;

    // Initialize captureConvDesc
    std::vector<uint8_t> captureConvData(bufLength, 0);
    BufferDesc captureConvDesc;
    captureConvDesc.bufLength = bufLength;
    captureConvDesc.buffer = captureConvData.data();
    float* rightBuff = reinterpret_cast<float*>(captureConvDesc.buffer);
    rightBuff[0] = 0.3f;

    std::shared_ptr<AudioEndpointInner> endpoint = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);
    std::vector<uint8_t> injectPeekBuffer(4);
    endpoint->injectPeekBuffer_ = injectPeekBuffer;
    float* mixBuff = endpoint->MixRendererAndCaptureData(bufLength, rendererConvDesc, captureConvDesc);

    EXPECT_EQ(mixBuff[0], 0.8f);
}

/**
 * @tc.name : Test CreateAndCfgLimiter API
 * @tc.type : FUNC
 * @tc.number: CreateAndCfgLimiter_001
 * @tc.desc : Test CreateAndCfgLimiter function with valid parameters.
 */
HWTEST(AudioEndpointInnerUnitTest, CreateAndCfgLimiter_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);

    // Set up initial state
    audioEndpointInner->limiter_ = std::make_shared<AudioLimiter>(HDI_INVALID_ID);

    const size_t bufLength = 1024;
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.channels = STEREO;

    int32_t result = audioEndpointInner->CreateAndCfgLimiter(bufLength, streamInfo);

    // Verify the result
    EXPECT_EQ(result, SUCCESS);
    audioEndpointInner->limiter_ = nullptr;
    result = audioEndpointInner->CreateAndCfgLimiter(bufLength, streamInfo);
    // Verify limiter configuration
    EXPECT_EQ(result, SUCCESS);
}

/**
@tc.name : Test ConvertDataFormat API
@tc.type : FUNC
@tc.number: ConvertDataFormat_001
@tc.desc : Test ConvertDataFormat function with valid parameters.
*/
HWTEST(AudioEndpointInnerUnitTest, ConvertDataFormat_001, TestSize.Level1)
{
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateInputEndpointInner(AudioEndpoint::TYPE_VOIP_MMAP);

    // Set up initial state
    audioEndpointInner->dstStreamInfo_.channels = STEREO;
    audioEndpointInner->dstStreamInfo_.format = SAMPLE_S16LE;
    audioEndpointInner->dstStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    audioEndpointInner->rendererConvBuffer_.assign(1024, 0);
    audioEndpointInner->captureConvBuffer_.assign(1024, 0);
    // Create input BufferDesc
    const size_t bufLength = 4 * sizeof(int16_t); // 4 samples
    BufferDesc readBuf = {new uint8_t[bufLength], bufLength};
    BufferDesc rendererOrgDesc = {new uint8_t[bufLength], bufLength};
    BufferDesc rendererConvDesc;
    BufferDesc captureConvDesc;

    // Set up stream info
    AudioStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.samplingRate = SAMPLE_RATE_48000;

    // Call the function
    int32_t result = audioEndpointInner->ConvertDataFormat(readBuf, rendererOrgDesc, streamInfo,
        rendererConvDesc, captureConvDesc);

    // Verify the result
    EXPECT_EQ(result, SUCCESS);

    // Cleanup
    delete[] readBuf.buffer;
    delete[] rendererOrgDesc.buffer;
}

/*
 * @tc.name  : Test CheckJank API
 * @tc.type  : FUNC
 * @tc.number: CheckJank_001
 * @tc.desc  : When isStarted_ is false, function should return directly
 */
HWTEST_F(AudioEndpointUnitTest, CheckJank_001, TestSize.Level1)
{
    int64_t currentTime = ClockTime::GetCurNano();
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->isStarted_ = false;
    audioEndpointInner->dstSpanSizeInframe_ = 0;
    audioEndpointInner->syncInfoSize_ = 1;
    audioEndpointInner->lastWriteTime_ = 0;
    audioEndpointInner->CheckJank(0);
    EXPECT_GT(currentTime, audioEndpointInner->lastWriteTime_);
}

/*
 * @tc.name  : Test CheckJank API
 * @tc.type  : FUNC
 * @tc.number: CheckJank_002
 * @tc.desc  : When isStarted_ is true but syncInfoSize_ is zero
 */
HWTEST_F(AudioEndpointUnitTest, CheckJank_002, TestSize.Level1)
{
    int64_t currentTime = ClockTime::GetCurNano();
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->isStarted_ = true;
    audioEndpointInner->dstSpanSizeInframe_ = 0;
    audioEndpointInner->syncInfoSize_ = 0;
    audioEndpointInner->lastWriteTime_ = 0;
    audioEndpointInner->CheckJank(0);
    EXPECT_GT(currentTime, audioEndpointInner->lastWriteTime_);
}

/*
 * @tc.name  : Test CheckJank API
 * @tc.type  : FUNC
 * @tc.number: CheckJank_003
 * @tc.desc  : When isStarted_ is true and syncInfoSize_ is non-zero
 */
HWTEST_F(AudioEndpointUnitTest, CheckJank_003, TestSize.Level1)
{
    int64_t currentTime = ClockTime::GetCurNano();
    std::shared_ptr<AudioEndpointInner> audioEndpointInner = CreateOutputEndpointInner(AudioEndpoint::TYPE_MMAP);
    audioEndpointInner->isStarted_ = true;
    audioEndpointInner->dstSpanSizeInframe_ = 0;
    audioEndpointInner->syncInfoSize_ = 1;
    audioEndpointInner->lastWriteTime_ = 0;
    audioEndpointInner->CheckJank(0);
    EXPECT_GT(audioEndpointInner->lastWriteTime_, currentTime);
}
} // namespace AudioStandard
} // namespace OHOS
