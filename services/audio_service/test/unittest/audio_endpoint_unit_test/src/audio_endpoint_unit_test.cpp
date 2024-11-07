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
#include "audio_utils.h"
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
    const AudioProcessConfig &clientConfig, const DeviceInfo &deviceInfo)
{
    std::shared_ptr<AudioEndpointInner> audioEndpoint = nullptr;
    if (type == AudioEndpoint::EndpointType::TYPE_INDEPENDENT && deviceInfo.deviceRole != INPUT_DEVICE &&
        deviceInfo.networkId == LOCAL_NETWORK_ID) {
        return nullptr;
    } else {
        audioEndpoint = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    }
    CHECK_AND_RETURN_RET_LOG(audioEndpoint != nullptr, nullptr, "Create AudioEndpoint failed.");

    if (!audioEndpoint->Config(deviceInfo)) {
        audioEndpoint = nullptr;
    }
    return audioEndpoint;
}

static std::shared_ptr<AudioEndpointInner> CreateInputEndpointInner(AudioEndpoint::EndpointType type)
{
    AudioProcessConfig config = {};
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::INPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpoint =
        std::make_shared<AudioEndpointInner>(type, AUDIO_ENDPOINT_ID, config);
    if (!audioEndpoint->Config(deviceInfo)) {
        audioEndpoint = nullptr;
    }
    return audioEndpoint;
}

static std::shared_ptr<AudioEndpointInner> CreateOutputEndpointInner(AudioEndpoint::EndpointType type)
{
    AudioProcessConfig config = {};
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::OUTPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.audioStreamInfo.channelLayout = CH_LAYOUT_STEREO;
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(type, AUDIO_ENDPOINT_ID, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpointInner);
    EXPECT_NE(nullptr, audioEndpointInner->fastSink_);

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
    DeviceInfo deviceInfo = {};
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    AudioProcessConfig serverConfig = InitServerProcessConfig();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(serverConfig, audioServicePtr);
    std::shared_ptr<OHAudioBuffer> buffer = nullptr;
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    processStream->ConfigProcessBuffer(totalSizeInFrame, spanSizeInFrame, deviceInfo.audioStreamInfo, buffer);
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
    DeviceInfo deviceInfo = {};
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.networkId = LOCAL_NETWORK_ID;
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
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::INPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.audioStreamInfo.format = AudioSampleFormat::SAMPLE_U8;
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    deviceInfo.audioStreamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    audioEndpointInner = CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    deviceInfo.audioStreamInfo.format = AudioSampleFormat::SAMPLE_S32LE;
    audioEndpointInner = CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    deviceInfo.audioStreamInfo.format = AudioSampleFormat::INVALID_WIDTH;
    audioEndpointInner = CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    deviceInfo.audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioEndpointInner = CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
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
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::INPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpoint =
        AudioEndpoint::CreateEndpoint(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpoint);
}

/*
 * @tc.name  : Test CreateEndpoint API
 * @tc.type  : FUNC
 * @tc.number: EnableCreateEndpoint_003
 * @tc.desc  : Test CreateEndpoint interface
 */
HWTEST_F(AudioEndpointUnitTest, EnableCreateEndpoint_003, TestSize.Level1)
{
    AudioProcessConfig config = {};
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::OUTPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpoint =
        AudioEndpoint::CreateEndpoint(AudioEndpoint::TYPE_INDEPENDENT, 123, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpoint);

    deviceInfo.networkId = REMOTE_NETWORK_ID;
    audioEndpoint = AudioEndpoint::CreateEndpoint(AudioEndpoint::TYPE_INDEPENDENT, 123, config, deviceInfo);
    EXPECT_EQ(nullptr, audioEndpoint);

    deviceInfo.deviceRole = DeviceRole::INPUT_DEVICE;
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    config.audioMode = AUDIO_MODE_RECORD;
    audioEndpoint = AudioEndpoint::CreateEndpoint(AudioEndpoint::TYPE_INDEPENDENT, 123, config, deviceInfo);
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
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::OUTPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpoint =
        AudioEndpoint::CreateEndpoint(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpoint);

    int32_t ret = audioEndpoint->EnableFastInnerCap();
    EXPECT_NE(SUCCESS, ret);

    audioEndpoint->Release();
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
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::INPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    deviceInfo.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string ret = audioEndpointInner->GenerateEndpointKey(deviceInfo, 1);
    EXPECT_NE("", ret);

    deviceInfo.deviceType = DEVICE_TYPE_INVALID;
    audioEndpointInner->GenerateEndpointKey(deviceInfo, 1);
    EXPECT_NE("", ret);

    audioEndpointInner->isInnerCapEnabled_ = true;
    int32_t result = audioEndpointInner->EnableFastInnerCap();
    EXPECT_EQ(SUCCESS, result);

    result = audioEndpointInner->DisableFastInnerCap();
    EXPECT_EQ(SUCCESS, result);

    audioEndpointInner->deviceInfo_.deviceRole = DeviceRole::OUTPUT_DEVICE;
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
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::OUTPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.audioStreamInfo.channelLayout = CH_LAYOUT_STEREO;
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpointInner);
    EXPECT_NE(nullptr, audioEndpointInner->fastSink_);

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
    audioEndpointInner->fastSink_ = nullptr;
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
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::OUTPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.audioStreamInfo.channelLayout = CH_LAYOUT_STEREO;
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
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
    audioEndpointInner->fastSink_ = nullptr;
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
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::INPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
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
 * @tc.name  : Test DfxOperation API
 * @tc.type  : FUNC
 * @tc.number: DfxOperation_001
 * @tc.desc  : Test DfxOperation interface
 */
HWTEST_F(AudioEndpointUnitTest, DfxOperation_001, TestSize.Level1)
{
    AudioProcessConfig config = {};
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::INPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    BufferDesc buffer = {};
    AudioSampleFormat format = SAMPLE_U8;
    AudioChannel channel = MONO;
    audioEndpointInner->DfxOperation(buffer, format, channel);
    EXPECT_EQ(MONO, channel);

    channel = STEREO;
    audioEndpointInner->DfxOperation(buffer, format, channel);
    EXPECT_EQ(STEREO, channel);
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
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::INPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
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
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::INPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->endpointType_ = AudioEndpoint::EndpointType::TYPE_INVALID;
    std::vector<AudioStreamData> audioDataList;
    AudioStreamData audioStreamInfo = {};
    audioEndpointInner->ProcessToDupStream(audioDataList, audioStreamInfo);

    audioEndpointInner->endpointType_ = AudioEndpoint::EndpointType::TYPE_VOIP_MMAP;
    audioStreamInfo.isInnerCaped = false;
    audioDataList.push_back(audioStreamInfo);
    audioEndpointInner->ProcessToDupStream(audioDataList, audioStreamInfo);
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
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::INPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpointInner> audioEndpointInner =
        CreateEndpointInner(AudioEndpoint::TYPE_MMAP, 123, config, deviceInfo);
    EXPECT_NE(nullptr, audioEndpointInner);

    deviceInfo.networkId = REMOTE_NETWORK_ID;
    IMmapAudioRendererSink *ret = audioEndpointInner->GetFastSink(deviceInfo, AudioEndpoint::TYPE_MMAP);
    EXPECT_NE(nullptr, ret);

    deviceInfo.networkId = LOCAL_NETWORK_ID;
    deviceInfo.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceInfo.a2dpOffloadFlag = A2DP_NOT_OFFLOAD;
    ret = audioEndpointInner->GetFastSink(deviceInfo, AudioEndpoint::TYPE_INVALID);
    EXPECT_NE(nullptr, ret);

    deviceInfo.networkId = LOCAL_NETWORK_ID;
    deviceInfo.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceInfo.a2dpOffloadFlag = A2DP_OFFLOAD;
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
    DeviceInfo deviceInfo = {};
    deviceInfo.deviceRole = DeviceRole::INPUT_DEVICE;
    deviceInfo.audioStreamInfo.samplingRate.insert(SAMPLE_RATE_48000);
    deviceInfo.audioStreamInfo.channels.insert(STEREO);
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    result = audioEndpointInner->Config(deviceInfo);
    EXPECT_FALSE(result);

    processStream->SetInnerCapState(true);
    result = audioEndpointInner->ShouldInnerCap();
    EXPECT_TRUE(result);

    processStream->SetInnerCapState(false);
    result = audioEndpointInner->ShouldInnerCap();
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

    audioEndpointInner->isInnerCapEnabled_ = true;
    EXPECT_TRUE(audioEndpointInner->StartDevice());

    EXPECT_TRUE(audioEndpointInner->StopDevice());

    audioEndpointInner->fastSource_->DeInit();
    audioEndpointInner->fastSource_ = nullptr;
    audioEndpointInner->isInnerCapEnabled_ = true;
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

    audioEndpointInner->fastSource_->DeInit();
    audioEndpointInner->fastSource_ = nullptr;
    audioEndpointInner->isInnerCapEnabled_ = true;
    EXPECT_FALSE(audioEndpointInner->DelayStopDevice());

    audioEndpointInner->deviceInfo_.deviceRole = OUTPUT_DEVICE;
    EXPECT_FALSE(audioEndpointInner->DelayStopDevice());
}
} // namespace AudioStandard
} // namespace OHOS
