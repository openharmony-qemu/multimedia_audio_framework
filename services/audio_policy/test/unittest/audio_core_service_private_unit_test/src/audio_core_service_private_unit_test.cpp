/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "audio_core_service_private_unit_test.h"
#include "hisysevent.h"
#include "audio_utils.h"
#include "mock_audio_arm_device_behavior.h"
#include "audio_device_factory.h"
#include "sco_audio_scene_manager.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t DEFAULT_PIPE_ID = 0;

static const int32_t BLUETOOTH_FETCH_RESULT_ERROR = 2;
static const uint32_t TEST_STREAM_1_SESSION_ID = 100001;
static const uint32_t TEST_STREAM_2_SESSION_ID = 100002;
static const uint32_t TEST_IO_HANDLE = 1234;
inline static const std::string EMPTY_ADDRESS{"00:00:00:00:00:00"};
inline static const std::string NULL_ADDRESS{""};

void AudioCoreServicePrivateTest::SetUp(void)
{
    testCoreService_ = std::make_shared<AudioCoreService>();
    testCoreService_->Init();
}

void AudioCoreServicePrivateTest::TearDown(void)
{
    testCoreService_ = nullptr;
}

static void MakeDeviceInfoMap(AudioPolicyConfigData &configData, std::shared_ptr<AdapterPipeInfo> &adapterPipeInfo,
    std::shared_ptr<PolicyAdapterInfo> &policyAdapterInfo)
{
    uint32_t routerFlag = AUDIO_OUTPUT_FLAG_LOWPOWER;
    AudioCoreConfigManager &manager = AudioCoreConfigManager::GetInstance();
    EXPECT_EQ(manager.Init(true), true);
    configData.Reorganize();

    policyAdapterInfo->adapterName = "remote";
    adapterPipeInfo->adapterInfo_ = policyAdapterInfo;
    std::shared_ptr<PipeStreamPropInfo> propInfo = std::make_shared<PipeStreamPropInfo>();
    propInfo->format_ = AudioSampleFormat::SAMPLE_S16LE;
    propInfo->sampleRate_ = AudioSamplingRate::SAMPLE_RATE_48000;
    propInfo->channels_ = AudioChannel::STEREO;
    propInfo->channelLayout_ = AudioChannelLayout::CH_LAYOUT_STEREO;
    propInfo->pipeInfo_ = adapterPipeInfo;

    std::shared_ptr<PipeStreamPropInfo> propInfo_2 = std::make_shared<PipeStreamPropInfo>();
    propInfo_2->format_ = AudioSampleFormat::SAMPLE_S16LE;
    propInfo_2->sampleRate_ = AudioSamplingRate::SAMPLE_RATE_48000;
    propInfo_2->channels_ = AudioChannel::CHANNEL_6;
    propInfo_2->channelLayout_ = AudioChannelLayout::CH_LAYOUT_5POINT1;
    propInfo_2->pipeInfo_ = adapterPipeInfo;

    std::shared_ptr<AdapterPipeInfo> info = std::make_shared<AdapterPipeInfo>();
    info->dynamicStreamPropInfos_ = {propInfo, propInfo_2};
    info->role_ = PIPE_ROLE_OUTPUT;

    std::shared_ptr<AdapterDeviceInfo> deviceInfo = std::make_shared<AdapterDeviceInfo>();
    deviceInfo->supportPipeMap_.insert({routerFlag, info});
    std::set<std::shared_ptr<AdapterDeviceInfo>> deviceInfoSet = {deviceInfo};
    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap[deviceKey] = deviceInfoSet;

    std::list<std::shared_ptr<PipeStreamPropInfo>> streamProps = {propInfo, propInfo_2};
    AudioPolicyConfigData::GetInstance().UpdateDynamicStreamProps("remote", "offload_distributed_output", streamProps);
}

static void MakeUidCheckDeviceInfoMap(AudioPolicyConfigData &configData, AudioFlag routerFlag, bool needUidCheck)
{
    AudioCoreConfigManager &manager = AudioCoreConfigManager::GetInstance();
    EXPECT_EQ(manager.Init(true), true);
    configData.Reorganize();

    std::shared_ptr<AdapterPipeInfo> info = std::make_shared<AdapterPipeInfo>();
    ASSERT_NE(info, nullptr);
    info->role_ = PIPE_ROLE_OUTPUT;
    info->needUidCheck_ = needUidCheck;

    std::shared_ptr<AdapterDeviceInfo> deviceInfo = std::make_shared<AdapterDeviceInfo>();
    ASSERT_NE(deviceInfo, nullptr);
    deviceInfo->supportPipeMap_.insert({routerFlag, info});

    std::set<std::shared_ptr<AdapterDeviceInfo>> deviceInfoSet = {deviceInfo};
    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap[deviceKey] = deviceInfoSet;
}

static void MakeUidCheckOffloadSupportDeviceInfoMap(AudioPolicyConfigData &configData, bool needUidCheck)
{
    AudioCoreConfigManager &manager = AudioCoreConfigManager::GetInstance();
    EXPECT_EQ(manager.Init(true), true);
    configData.Reorganize();

    std::shared_ptr<PipeStreamPropInfo> propInfo = std::make_shared<PipeStreamPropInfo>();
    ASSERT_NE(propInfo, nullptr);
    propInfo->format_ = AudioSampleFormat::SAMPLE_S16LE;
    propInfo->sampleRate_ = AudioSamplingRate::SAMPLE_RATE_48000;
    propInfo->channelLayout_ = AudioChannelLayout::CH_LAYOUT_STEREO;

    std::shared_ptr<AdapterPipeInfo> info = std::make_shared<AdapterPipeInfo>();
    ASSERT_NE(info, nullptr);
    info->role_ = PIPE_ROLE_OUTPUT;
    info->supportFlags_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    info->needUidCheck_ = needUidCheck;
    info->streamPropInfos_.push_back(propInfo);

    std::shared_ptr<AdapterDeviceInfo> deviceInfo = std::make_shared<AdapterDeviceInfo>();
    ASSERT_NE(deviceInfo, nullptr);
    deviceInfo->supportPipeMap_.insert({AUDIO_OUTPUT_FLAG_LOWPOWER, info});

    std::set<std::shared_ptr<AdapterDeviceInfo>> deviceInfoSet = {deviceInfo};
    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap[deviceKey] = deviceInfoSet;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_001
 * @tc.desc  : Test AudioCoreService::GetEncryptAddr()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::string addr = "abc";

    auto ret = audioCoreService->GetEncryptAddr(addr);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_002
 * @tc.desc  : Test AudioCoreService::GetEncryptAddr()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::string addr = "";

    auto ret = audioCoreService->GetEncryptAddr(addr);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_003
 * @tc.desc  : Test AudioCoreService::GetEncryptAddr()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::string addr = "12345678901234567";

    auto ret = audioCoreService->GetEncryptAddr(addr);
    EXPECT_EQ(ret, "123456**:**:**567");
}

/**
 * @tc.name  : Test ScoInputDeviceFetchedForRecongnition.
 * @tc.number: ScoInputDeviceFetchedForRecongnition_001
 * @tc.desc  : Test AudioCoreService::ScoInputDeviceFetchedForRecongnition()
 */
HWTEST_F(AudioCoreServicePrivateTest, ScoInputDeviceFetchedForRecongnition_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    bool handleFlag = false;
    std::string address = "abc";
    ConnectState connectState = DEACTIVE_CONNECTED;
    bool isVrSupported = false;

    auto ret = audioCoreService->ScoInputDeviceFetchedForRecongnition(
        handleFlag, address, connectState, isVrSupported);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ScoInputDeviceFetchedForRecongnition.
 * @tc.number: ScoInputDeviceFetchedForRecongnition_002
 * @tc.desc  : Test AudioCoreService::ScoInputDeviceFetchedForRecongnition()
 */
HWTEST_F(AudioCoreServicePrivateTest, ScoInputDeviceFetchedForRecongnition_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    bool handleFlag = true;
    std::string address = "abc";
    ConnectState connectState = DEACTIVE_CONNECTED;
    bool isVrSupported = false;

    auto ret = audioCoreService->ScoInputDeviceFetchedForRecongnition(
        handleFlag, address, connectState, isVrSupported);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ScoInputDeviceFetchedForRecongnition.
 * @tc.number: ScoInputDeviceFetchedForRecongnition_003
 * @tc.desc  : Test AudioCoreService::ScoInputDeviceFetchedForRecongnition()
 */
HWTEST_F(AudioCoreServicePrivateTest, ScoInputDeviceFetchedForRecongnition_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    bool handleFlag = true;
    std::string address = "abc";
    ConnectState connectState = CONNECTED;
    bool isVrSupported = false;

    auto ret = audioCoreService->ScoInputDeviceFetchedForRecongnition(
        handleFlag, address, connectState, isVrSupported);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ScoInputDeviceFetchedForRecongnition.
 * @tc.number: ScoInputDeviceFetchedForRecongnition_004
 * @tc.desc  : Test AudioCoreService::ScoInputDeviceFetchedForRecongnition()
 */
HWTEST_F(AudioCoreServicePrivateTest, ScoInputDeviceFetchedForRecongnition_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    bool handleFlag = true;
    std::string address = "abc";
    ConnectState connectState = DEACTIVE_CONNECTED;
    bool isVrSupported = true;

    auto ret = audioCoreService->ScoInputDeviceFetchedForRecongnition(
        handleFlag, address, connectState, isVrSupported);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ScoInputDeviceFetchedForRecongnition.
 * @tc.number: ScoInputDeviceFetchedForRecongnition_005
 * @tc.desc  : Test AudioCoreService::ScoInputDeviceFetchedForRecongnition()
 */
HWTEST_F(AudioCoreServicePrivateTest, ScoInputDeviceFetchedForRecongnition_005, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    bool handleFlag = true;
    std::string address = "abc";
    ConnectState connectState = CONNECTED;
    bool isVrSupported = true;

    auto ret = audioCoreService->ScoInputDeviceFetchedForRecongnition(
        handleFlag, address, connectState, isVrSupported);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_009
 * @tc.desc  : Test AudioCoreService::CheckModemScene()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_009, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_010
 * @tc.desc  : Test AudioCoreService::CheckModemScene()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_010, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    audioCoreService->CheckModemScene(modemDescs, reason);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: AudioCoreServicePrivate_027
* @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_027, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_NONE;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_NONE;
    deviceInfo.macAddress_ = "macAddress";
    deviceInfo.connectState_ = CONNECTED;
    deviceInfo.descriptorType_ = AudioDeviceDescriptor::AUDIO_DEVICE_DESCRIPTOR;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_028
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_028, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceInfo.macAddress_ = "macAddress";
    deviceInfo.connectState_ = CONNECTED;
    deviceInfo.descriptorType_ = AudioDeviceDescriptor::DEVICE_INFO;
    deviceInfo.a2dpOffloadFlag_ = A2DP_OFFLOAD;

    audioCoreService->audioA2dpOffloadFlag_.a2dpOffloadFlag_ = NO_A2DP_DEVICE;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_029
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_029, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_INVALID;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_INVALID;
    deviceInfo.macAddress_ = "macAddress";
    deviceInfo.connectState_ = CONNECTED;
    deviceInfo.descriptorType_ = AudioDeviceDescriptor::DEVICE_INFO;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_030
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_030, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;
    desc->deviceRole_ = DEVICE_ROLE_NONE;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    deviceInfo.macAddress_ = "macAddress";
    deviceInfo.connectState_ = CONNECTED;
    deviceInfo.descriptorType_ = AudioDeviceDescriptor::DEVICE_INFO;
    deviceInfo.deviceRole_ = DEVICE_ROLE_NONE;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_031
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_031, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    deviceInfo.macAddress_ = "macAddress";
    deviceInfo.connectState_ = SUSPEND_CONNECTED;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_032
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_032, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    deviceInfo.macAddress_ = "abc";
    deviceInfo.connectState_ = SUSPEND_CONNECTED;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_033
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_033, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_DP;
    deviceInfo.macAddress_ = "abc";
    deviceInfo.connectState_ = SUSPEND_CONNECTED;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_034
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_034, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "abc";
    deviceInfo.deviceType_ = DEVICE_TYPE_DP;
    deviceInfo.macAddress_ = "abc";
    deviceInfo.connectState_ = SUSPEND_CONNECTED;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_037
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeNew()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_037, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeNew(pipeInfo, flag, reason);
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_038
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeNew()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_038, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_MOVE;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeNew(pipeInfo, flag, reason);
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_039
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeNew()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_039, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_RECREATE;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeNew(pipeInfo, flag, reason);
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_040
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeNew()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_040, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeNew(pipeInfo, flag, reason);
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_049
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_049, TestSize.Level1)
{
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(DEVICE_TYPE_INVALID, true);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_050
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_050, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_EARPIECE;

    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_051
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_051, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_052
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_052, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;

    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_053
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_053, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADPHONES;

    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_054
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_054, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_055
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_055, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_DP;
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_056
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_056, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_057
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_057, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_058
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_058, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_DEFAULT;
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_059
 * @tc.desc : Test AudioCoreService::GetRealUid
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_059, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerUid_ = 1013;
    streamDesc->appInfo_.appUid = 1013;

    auto audioCoreService = AudioCoreService::GetCoreService();

    int32_t result = audioCoreService->GetRealUid(streamDesc);

    EXPECT_EQ(result, 1013);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_060
 * @tc.desc : Test AudioCoreService::GetRealUid
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_060, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto audioCoreService = AudioCoreService::GetCoreService();

    streamDesc->callerUid_ = 0;

    int32_t result = audioCoreService->GetRealUid(streamDesc);

    EXPECT_EQ(result, 0);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_GetRealPid
 * @tc.desc : Test AudioCoreService::GetRealPid
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_GetRealPid, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerUid_ = 1013;
    streamDesc->callerPid_ = 1013;
    streamDesc->appInfo_.appPid = 1013;

    auto audioCoreService = AudioCoreService::GetCoreService();

    int32_t result = audioCoreService->GetRealPid(streamDesc);

    EXPECT_EQ(result, 1013);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_GetRealPid_02
 * @tc.desc : Test AudioCoreService::GetRealPid
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_GetRealPid_02, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto audioCoreService = AudioCoreService::GetCoreService();

    streamDesc->callerPid_ = 0;

    int32_t result = audioCoreService->GetRealPid(streamDesc);

    EXPECT_EQ(result, 0);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_061
 * @tc.desc : Test AudioCoreService::UpdateRendererInfoWhenNoPermission
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_061, TestSize.Level1)
{
    auto audioRendererChangeInfos = std::make_shared<AudioRendererChangeInfo>();
    bool hasSystemPermission = true;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->UpdateRendererInfoWhenNoPermission(audioRendererChangeInfos, hasSystemPermission);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_062
 * @tc.desc : Test AudioCoreService::UpdateRendererInfoWhenNoPermission
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_062, TestSize.Level1)
{
    auto audioRendererChangeInfos = std::make_shared<AudioRendererChangeInfo>();
    bool hasSystemPermission = false;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->UpdateRendererInfoWhenNoPermission(audioRendererChangeInfos, hasSystemPermission);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_063
 * @tc.desc : Test AudioCoreService::UpdateRendererInfoWhenNoPermission
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_063, TestSize.Level1)
{
    auto audioCapturerChangeInfos = std::make_shared<AudioCapturerChangeInfo>();
    bool hasSystemPermission = true;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->UpdateCapturerInfoWhenNoPermission(audioCapturerChangeInfos, hasSystemPermission);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_064
 * @tc.desc : Test AudioCoreService::UpdateRendererInfoWhenNoPermission
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_064, TestSize.Level1)
{
    auto audioCapturerChangeInfos = std::make_shared<AudioCapturerChangeInfo>();
    bool hasSystemPermission = false;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->UpdateCapturerInfoWhenNoPermission(audioCapturerChangeInfos, hasSystemPermission);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_065
 * @tc.desc : Test AudioUtils::GetFastControlParam
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_065, TestSize.Level1)
{
    SetSysPara("persist.multimedia.audioflag.fastcontrolled", 0);

    bool result = GetFastControlParam();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_066
 * @tc.desc : Test AudioUtils::GetFastControlParam
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_066, TestSize.Level1)
{
    SetSysPara("persist.multimedia.audioflag.fastcontrolled", 1);

    bool result = GetFastControlParam();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_067
 * @tc.desc : Test AudioCoreService::NeedRehandleA2DPDevice
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_067, TestSize.Level1)

{
    auto desc = std::make_shared<AudioDeviceDescriptor>();

    auto audioCoreService = AudioCoreService::GetCoreService();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;

    bool result = audioCoreService->NeedRehandleA2DPDevice(desc);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_068
 * @tc.desc : Test AudioCoreService::NeedRehandleA2DPDevice
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_068, TestSize.Level1)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();

    auto audioCoreService = AudioCoreService::GetCoreService();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string moduleName = BLUETOOTH_SPEAKER;
    AudioIOHandle moduleId = 0;

    audioCoreService->audioIOHandleMap_.AddIOHandleInfo(moduleName, moduleId);
    bool result = audioCoreService->NeedRehandleA2DPDevice(desc);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_069
 * @tc.desc : Test AudioCoreService::NeedRehandleA2DPDevice
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_069, TestSize.Level1)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();

    auto audioCoreService = AudioCoreService::GetCoreService();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string moduleName = BLUETOOTH_MIC;
    AudioIOHandle moduleId = 0;

    audioCoreService->audioIOHandleMap_.AddIOHandleInfo(moduleName, moduleId);
    bool result = audioCoreService->NeedRehandleA2DPDevice(desc);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_070
 * @tc.desc : Test AudioCoreService::TriggerRecreateRendererStreamCallback
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_070, TestSize.Level1)
{
    shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerPid_ = 0;
    streamDesc->sessionId_ = 0;
    streamDesc->routeFlag_ = true;
    streamDesc->rendererInfo_.isOffloadAllowed = true;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->rendererInfo_.playerType = PLAYER_TYPE_SOUND_POOL;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->oldDeviceDescs_.push_back(deviceDesc);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        AudioStreamDeviceChangeReasonExt::ExtEnum::SET_DEFAULT_OUTPUT_DEVICE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);

    auto audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->SetCallbackHandler(nullptr);

    audioCoreService->TriggerRecreateRendererStreamCallback(streamDesc, reason);
    EXPECT_EQ(audioCoreService->audioPolicyServerHandler_, nullptr);

    bool isSupportLowPower = audioCoreService->IsStreamSupportLowpower(streamDesc);
    EXPECT_EQ(isSupportLowPower, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_071
 * @tc.desc : Test AudioCoreService::TriggerRecreateRendererStreamCallback
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_071, TestSize.Level1)
{
    shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerPid_ = 0;
    streamDesc->sessionId_ = 0;
    streamDesc->routeFlag_ = true;
    streamDesc->rendererInfo_.isOffloadAllowed = true;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->rendererInfo_.playerType = PLAYER_TYPE_SOUND_POOL;
    std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->oldDeviceDescs_.push_back(oldDeviceDesc);
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    newDeviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        AudioStreamDeviceChangeReasonExt::ExtEnum::SET_DEFAULT_OUTPUT_DEVICE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);

    auto audioCoreService = AudioCoreService::GetCoreService();
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    audioCoreService->SetCallbackHandler(handler);

    audioCoreService->TriggerRecreateRendererStreamCallback(streamDesc, reason);
    EXPECT_NE(audioCoreService->audioPolicyServerHandler_, nullptr);

    bool isSupportLowPower = audioCoreService->IsStreamSupportLowpower(streamDesc);
    EXPECT_EQ(isSupportLowPower, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_072
 * @tc.desc : Test AudioCoreService::TriggerRecreateCapturerStreamCallback
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_072, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 0,
    streamDesc->callerUid_ = 0;
    streamDesc->appInfo_.appUid = 0;
    streamDesc->appInfo_.appPid = 0;
    streamDesc->appInfo_.appTokenId = 0;
    streamDesc->streamStatus_ = STREAM_STATUS_NEW;
    streamDesc->routeFlag_ = true;

    auto audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->SetCallbackHandler(nullptr);

    audioCoreService->TriggerRecreateCapturerStreamCallback(streamDesc);

    EXPECT_EQ(audioCoreService->audioPolicyServerHandler_, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_073
 * @tc.desc : Test AudioCoreService::TriggerRecreateCapturerStreamCallback
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_073, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 123456,
    streamDesc->callerUid_ = 0;
    streamDesc->appInfo_.appUid = 0;
    streamDesc->appInfo_.appPid = 0;
    streamDesc->appInfo_.appTokenId = 0;
    streamDesc->streamStatus_ = STREAM_STATUS_NEW;
    streamDesc->routeFlag_ = true;

    auto audioCoreService = AudioCoreService::GetCoreService();
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    audioCoreService->SetCallbackHandler(handler);

    audioCoreService->TriggerRecreateCapturerStreamCallback(streamDesc);
    EXPECT_NE(audioCoreService->audioPolicyServerHandler_, nullptr);

    bool ret = SwitchStreamUtil::RemoveAllRecordBySessionId(123456);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_074
 * @tc.desc : Test AudioCoreService::HandleStreamStatusToCapturerState
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_074, TestSize.Level2)
{
    auto audioCoreService = AudioCoreService::GetCoreService();

    CapturerState state = audioCoreService->HandleStreamStatusToCapturerState(STREAM_STATUS_NEW);
    EXPECT_EQ(state, CAPTURER_PREPARED);

    state = audioCoreService->HandleStreamStatusToCapturerState(STREAM_STATUS_STARTED);
    EXPECT_EQ(state, CAPTURER_RUNNING);

    state = audioCoreService->HandleStreamStatusToCapturerState(STREAM_STATUS_PAUSED);
    EXPECT_EQ(state, CAPTURER_PAUSED);

    state = audioCoreService->HandleStreamStatusToCapturerState(STREAM_STATUS_STOPPED);
    EXPECT_EQ(state, CAPTURER_STOPPED);

    state = audioCoreService->HandleStreamStatusToCapturerState(STREAM_STATUS_RELEASED);
    EXPECT_EQ(state, CAPTURER_RELEASED);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_075
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_075, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_LINE_DIGITAL;
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_076
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_076, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_WIRED_MICIN;
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_100
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeUpdate()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_100, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    audioStreamDescriptor->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeUpdate(pipeInfo, flag, reason);
    EXPECT_EQ(flag, AUDIO_OUTPUT_FLAG_DIRECT);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_101
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeUpdate()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_101, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeUpdate(pipeInfo, flag, reason);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_102
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeUpdate()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_102, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_RECREATE;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    pipeInfo->moduleInfo_.name = BLUETOOTH_MIC;

    uint32_t flag = 0;

    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeUpdate(pipeInfo, flag, reason);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_103
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeUpdate()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_103, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(5);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    pipeInfo->moduleInfo_.name = OFFLOAD_PRIMARY_SPEAKER;

    uint32_t flag = 0;

    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeUpdate(pipeInfo, flag, reason);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_105
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeNew()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_105, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeNew(pipeInfo, flag);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_106
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeNew()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_106, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_RECREATE;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeNew(pipeInfo, flag);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_107
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeNew()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_107, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(5);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeNew(pipeInfo, flag);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_108
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeUpdate()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_108, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    audioStreamDescriptor->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeUpdate(pipeInfo, flag);
    EXPECT_EQ(flag, AUDIO_OUTPUT_FLAG_DIRECT);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_109
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeUpdate()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_109, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeUpdate(pipeInfo, flag);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_110
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeUpdate()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_110, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_RECREATE;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeUpdate(pipeInfo, flag);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_111
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeUpdate()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_111, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(5);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeUpdate(pipeInfo, flag);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_115
 * @tc.desc  : Test AudioCoreService::MoveToNewInputDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_115, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    streamDesc->oldDeviceDescs_.clear();
    EXPECT_EQ(streamDesc->oldDeviceDescs_.size(), 0);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::vector<SourceOutput> sourceOutputs = audioCoreService->GetSourceOutputs();

    audioCoreService->MoveToNewInputDevice(streamDesc, sourceOutputs);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_116
 * @tc.desc  : Test AudioCoreService::MoveToNewInputDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_116, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->oldDeviceDescs_.push_back(audioDeviceDescriptor);
    EXPECT_NE(streamDesc->oldDeviceDescs_.size(), 0);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor2 = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor2);
    std::vector<SourceOutput> sourceOutputs = audioCoreService->GetSourceOutputs();

    audioCoreService->MoveToNewInputDevice(streamDesc, sourceOutputs);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_117
 * @tc.desc  : Test AudioCoreService::IsNewDevicePlaybackSupported()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_117, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    bool ret = audioCoreService->IsNewDevicePlaybackSupported(streamDesc);
    EXPECT_EQ(ret, false);

    streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    ret = audioCoreService->IsNewDevicePlaybackSupported(streamDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_118
 * @tc.desc  : Test AudioCoreService::IsNewDevicePlaybackSupported()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_118, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    streamDesc->newDeviceDescs_.push_back(nullptr);
    bool ret = audioCoreService->IsNewDevicePlaybackSupported(streamDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_119
 * @tc.desc  : Test AudioCoreService::IsNewDevicePlaybackSupported()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_119, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(newDeviceDesc, nullptr);
    newDeviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);
    streamDesc->streamInfo_.encoding = ENCODING_EAC3;

    bool ret = audioCoreService->IsNewDevicePlaybackSupported(streamDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_120
 * @tc.desc  : Test AudioCoreService::IsNewDevicePlaybackSupported()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_120, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(newDeviceDesc, nullptr);
    newDeviceDesc->deviceType_ = DEVICE_TYPE_HDMI;
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);
    streamDesc->streamInfo_.encoding = ENCODING_EAC3;

    bool ret = audioCoreService->IsNewDevicePlaybackSupported(streamDesc);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsFastAllowedTest_001
 * @tc.desc  : Test AudioCoreService::IsFastAllowed, return true when bundleName is null.
 */
HWTEST_F(AudioCoreServicePrivateTest, IsFastAllowedTest_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::string bundleName = "";
    EXPECT_EQ(audioCoreService->IsFastAllowed(bundleName), true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsFastAllowedTest_002
 * @tc.desc  : Test AudioCoreService::IsFastAllowed, return true when bundleName is normal app.
 */
HWTEST_F(AudioCoreServicePrivateTest, IsFastAllowedTest_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::string bundleName = "com.example.app";
    streamDesc->SetBundleName(bundleName);
    EXPECT_EQ(audioCoreService->IsFastAllowed(streamDesc->bundleName_), true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: HandleFetchOutputWhenNoRunningStream_001
 * @tc.desc  : Test AudioCoreService::HandleFetchOutputWhenNoRunningStream, fetch output when no running stream.
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleFetchOutputWhenNoRunningStream_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    auto ret = audioCoreService->HandleFetchOutputWhenNoRunningStream(AudioStreamDeviceChangeReason::UNKNOWN);
    EXPECT_EQ(ret, SUCCESS);
}
/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: HandleFetchOutputWhenNoRunningStream_003
 * @tc.desc  : Test AudioCoreService::HandleFetchOutputWhenNoRunningStream, fetch output when no running stream.
 */

HWTEST_F(AudioCoreServicePrivateTest, HandleFetchOutputWhenNoRunningStream_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(desc, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(deviceDesc, nullptr);
    deviceDesc->deviceType_ = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;
    desc->newDeviceDescs_.push_back(deviceDesc);

    auto ret = audioCoreService->HandleFetchOutputWhenNoRunningStream(AudioStreamDeviceChangeReason::UNKNOWN);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: HandleFetchOutputWhenNoRunningStream_004
 * @tc.desc  : Test AudioCoreService::HandleFetchOutputWhenNoRunningStream, fetch output when no running stream.
 */

HWTEST_F(AudioCoreServicePrivateTest, HandleFetchOutputWhenNoRunningStream_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    auto ret = audioCoreService->HandleFetchOutputWhenNoRunningStream(
        AudioStreamDeviceChangeReasonExt::ExtEnum::SET_AUDIO_SCENE);
    EXPECT_EQ(ret, SUCCESS);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    ret = audioCoreService->HandleFetchOutputWhenNoRunningStream(
        AudioStreamDeviceChangeReasonExt::ExtEnum::SET_AUDIO_SCENE);
    EXPECT_EQ(ret, SUCCESS);
}

constexpr int32_t TEST_DEVICE_ID = 1001;
/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_001
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to started if scene is AUDIO_SCENE_PHONE_CALL
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckModemScene_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STARTED);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_002
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to stopped if scene is AUDIO_SCENE_DEFAULT
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckModemScene_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STOPPED);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_003
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to started if scene is AUDIO_SCENE_PHONE_CALL
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckModemScene_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STARTED);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_004
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to stopped if scene is AUDIO_SCENE_DEFAULT
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckModemScene_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STOPPED);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_005
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to started if scene is AUDIO_SCENE_PHONE_CALL
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckModemScene_005, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STARTED);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_006
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to stopped if scene is AUDIO_SCENE_DEFAULT
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckModemScene_006, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STOPPED);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_007
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to started if scene is AUDIO_SCENE_PHONE_CALL
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckModemScene_007, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STARTED);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_008
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to stopped if scene is AUDIO_SCENE_DEFAULT
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckModemScene_008, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STOPPED);
}


/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: UpdateModemRoute_001
 * @tc.desc  : Test AudioCoreService::UpdateModemRoute
 */
HWTEST_F(AudioCoreServicePrivateTest, UpdateModemRoute_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    FetchDeviceInfo info = { STREAM_USAGE_VOICE_MODEM_COMMUNICATION, "" };
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs =
        audioCoreService->audioRouterCenter_.FetchOutputDevices(info);

    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    int32_t ret = audioCoreService->UpdateModemRoute(modemDescs);
    EXPECT_EQ(ret, SUCCESS);

    audioCoreService->needUnmuteVoiceCall_ = true;
    ret = audioCoreService->UpdateModemRoute(modemDescs);
    EXPECT_EQ(ret, SUCCESS);

    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: UpdateModemRoute_002
 * @tc.desc  : Test AudioCoreService::UpdateModemRoute
 */
HWTEST_F(AudioCoreServicePrivateTest, UpdateModemRoute_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    FetchDeviceInfo info = { STREAM_USAGE_VOICE_MODEM_COMMUNICATION, "" };
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs =
        audioCoreService->audioRouterCenter_.FetchOutputDevices(info);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    int32_t ret = audioCoreService->UpdateModemRoute(modemDescs);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: GetVoiceCallMuteDuration_001
 * @tc.desc  : Test AudioCoreService::GetVoiceCallMuteDuration
 */
HWTEST_F(AudioCoreServicePrivateTest, GetVoiceCallMuteDuration_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    AudioDeviceDescriptor desc1(DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    AudioDeviceDescriptor desc2(DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    AudioDeviceDescriptor desc3(DeviceType::DEVICE_TYPE_USB_HEADSET, DeviceRole::OUTPUT_DEVICE);
    AudioDeviceDescriptor desc4(DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    desc4.networkId_ = "RemoteDevice";
    uint32_t targetMuteDuration = 100000;

    uint32_t muteDuration = audioCoreService->GetVoiceCallMuteDuration(desc1, desc1);
    EXPECT_EQ(muteDuration, 0);

    muteDuration = audioCoreService->GetVoiceCallMuteDuration(desc1, desc2);
    EXPECT_EQ(muteDuration, 0);

    muteDuration = audioCoreService->GetVoiceCallMuteDuration(desc1, desc3);
    EXPECT_EQ(muteDuration, targetMuteDuration);

    muteDuration = audioCoreService->GetVoiceCallMuteDuration(desc3, desc1);
    EXPECT_EQ(muteDuration, targetMuteDuration);

    muteDuration = audioCoreService->GetVoiceCallMuteDuration(desc1, desc4);
    EXPECT_EQ(muteDuration, targetMuteDuration);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsStreamSupportLowpower_001
 * @tc.desc  : Test AudioCoreService::IsStreamSupportLowpower, if playerType is PLAYER_TYPE_SOUND_POOL, return false
 */
HWTEST_F(AudioCoreServicePrivateTest, IsStreamSupportLowpower_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.isOffloadAllowed = true;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->rendererInfo_.playerType = PLAYER_TYPE_SOUND_POOL;
    bool isSupportLowPower = audioCoreService->IsStreamSupportLowpower(streamDesc);
    EXPECT_EQ(isSupportLowPower, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsStreamSupportLowpower_002
 * @tc.desc  : Test AudioCoreService::IsStreamSupportLowpower, if playerType is PLAYER_TYPE_OPENSL_ES, return false
 */
HWTEST_F(AudioCoreServicePrivateTest, IsStreamSupportLowpower_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.isOffloadAllowed = true;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->rendererInfo_.playerType = PLAYER_TYPE_OPENSL_ES;
    bool isSupportLowPower = audioCoreService->IsStreamSupportLowpower(streamDesc);
    EXPECT_EQ(isSupportLowPower, false);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: IsStreamSupportLowpower_003
* @tc.desc  : Test interface IsStreamSupportLowpower
*/
HWTEST_F(AudioCoreServicePrivateTest, IsStreamSupportLowpower_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest IsStreamSupportLower start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(streamDesc, nullptr);
    const int32_t AUDIO_EXT_UID = 1041;
    streamDesc->rendererInfo_.isOffloadAllowed = true;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->callerUid_ = AUDIO_EXT_UID;
    bool isSupportLowPower = audioCoreService->IsStreamSupportLowpower(streamDesc);
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest IsStreamSupportLower_003 end");
    EXPECT_EQ(isSupportLowPower, false);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: PauseClient_002
* @tc.desc  : Test PauseClient
*/
HWTEST_F(AudioCoreServicePrivateTest, PauseClient_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 123;
    desc->audioMode_ == AUDIO_MODE_PLAYBACK;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_NE(nullptr, audioCoreService);
    uint32_t targetSessionId = 123;
    auto ret = audioCoreService->PauseClient(targetSessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: PauseClient_003
* @tc.desc  : Test PauseClient
*/
HWTEST_F(AudioCoreServicePrivateTest, PauseClient_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 123;
    desc->audioMode_ == AUDIO_MODE_RECORD;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_NE(nullptr, audioCoreService);
    uint32_t targetSessionId = 123;
    auto ret = audioCoreService->PauseClient(targetSessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: StandbyClient_001
* @tc.desc  : Test StandbyClient
*/
HWTEST_F(AudioCoreServicePrivateTest, StandbyClient_001, TestSize.Level1)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_NE(nullptr, audioCoreService);
    auto ret = audioCoreService->StandbyClient(1000);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: StopClient_002
* @tc.desc  : Test StopClient
*/
HWTEST_F(AudioCoreServicePrivateTest, StopClient_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 123;
    desc->audioMode_ == AUDIO_MODE_PLAYBACK;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_NE(nullptr, audioCoreService);
    uint32_t targetSessionId = 123;
    auto ret = audioCoreService->StopClient(targetSessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: StopClient_003
* @tc.desc  : Test StopClient
*/
HWTEST_F(AudioCoreServicePrivateTest, StopClient_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 123;
    desc->audioMode_ == AUDIO_MODE_RECORD;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_NE(nullptr, audioCoreService);
    uint32_t targetSessionId = 123;
    auto ret = audioCoreService->StopClient(targetSessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: ReleaseClient_002
* @tc.desc  : Test ReleaseClient
*/
HWTEST_F(AudioCoreServicePrivateTest, ReleaseClient_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 123;
    desc->audioMode_ == AUDIO_MODE_PLAYBACK;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_NE(nullptr, audioCoreService);
    uint32_t targetSessionId = 123;
    auto ret = audioCoreService->ReleaseClient(targetSessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: ReleaseClient_003
* @tc.desc  : Test ReleaseClient
*/
HWTEST_F(AudioCoreServicePrivateTest, ReleaseClient_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 123;
    desc->audioMode_ == AUDIO_MODE_RECORD;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_NE(nullptr, audioCoreService);
    uint32_t targetSessionId = 123;
    auto ret = audioCoreService->ReleaseClient(targetSessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: IsStreamSupportLowpower_004
* @tc.desc  : Test interface IsStreamSupportLowpower
*/
HWTEST_F(AudioCoreServicePrivateTest, IsStreamSupportLowpower_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest IsStreamSupportLower start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(streamDesc, nullptr);
    const int32_t MEDIA_SERVICE_UID = 1013;
    const int32_t AUDIO_EXT_UID = 1041;
    streamDesc->rendererInfo_.isOffloadAllowed = true;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->callerUid_ = MEDIA_SERVICE_UID;
    streamDesc->appInfo_.appUid = AUDIO_EXT_UID;
    bool isSupportLowPower = audioCoreService->IsStreamSupportLowpower(streamDesc);
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest IsStreamSupportLower_004 end");
    EXPECT_EQ(isSupportLowPower, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: OffloadAllowedForUid_001
 * @tc.desc  : Test AudioCoreService::SetOffloadAllowedForUid and IsOffloadAllowedForUid
 */
HWTEST_F(AudioCoreServicePrivateTest, OffloadAllowedForUid_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    constexpr uint32_t TEST_UID = 1905;
    EXPECT_EQ(audioCoreService->IsOffloadAllowedForUid(TEST_UID), false);
    EXPECT_EQ(audioCoreService->SetOffloadAllowedForUid(TEST_UID, true), SUCCESS);
    EXPECT_EQ(audioCoreService->IsOffloadAllowedForUid(TEST_UID), true);
    EXPECT_EQ(audioCoreService->SetOffloadAllowedForUid(TEST_UID, false), SUCCESS);
    EXPECT_EQ(audioCoreService->IsOffloadAllowedForUid(TEST_UID), false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: NeedUidCheckForOffloadPipe_001
 * @tc.desc  : Test AudioCoreConfigManager::NeedUidCheckForOffloadPipe with combined lowpower flag
 */
HWTEST_F(AudioCoreServicePrivateTest, NeedUidCheckForOffloadPipe_001, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    MakeUidCheckDeviceInfoMap(configData,
        static_cast<AudioFlag>(AUDIO_OUTPUT_FLAG_LOWPOWER | AUDIO_OUTPUT_FLAG_DIRECT), true);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    ASSERT_NE(deviceDesc, nullptr);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    bool needUidCheck = AudioCoreConfigManager::GetInstance().NeedUidCheckForOffloadPipe(streamDesc);
    EXPECT_EQ(needUidCheck, true);

    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap.erase(deviceKey);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsOffloadPlaybackSupported_001
 * @tc.desc  : Test AudioCoreConfigManager::IsOffloadPlaybackSupported returns false for uid-gated pipe
 */
HWTEST_F(AudioCoreServicePrivateTest, IsOffloadPlaybackSupported_001, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    MakeUidCheckOffloadSupportDeviceInfoMap(configData, true);

    AudioStreamInfo streamInfo {};
    streamInfo.format = AudioSampleFormat::INVALID_WIDTH;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_UNKNOWN;

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    ASSERT_NE(deviceDesc, nullptr);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc = {deviceDesc};

    bool isSupported = AudioCoreConfigManager::GetInstance().IsOffloadPlaybackSupported(
        streamInfo, desc, STREAM_USAGE_MUSIC);
    EXPECT_EQ(isSupported, false);

    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap.erase(deviceKey);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckOffloadPipeUidAllowed_001
 * @tc.desc  : Test AudioCoreService::CheckOffloadPipeUidAllowed returns true when uid check is not needed
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckOffloadPipeUidAllowed_001, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    MakeUidCheckDeviceInfoMap(configData, AUDIO_OUTPUT_FLAG_LOWPOWER, false);

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    ASSERT_NE(deviceDesc, nullptr);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    EXPECT_EQ(audioCoreService->CheckOffloadPipeUidAllowed(streamDesc, 200100), true);

    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap.erase(deviceKey);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckOffloadPipeUidAllowed_002
 * @tc.desc  : Test AudioCoreService::CheckOffloadPipeUidAllowed returns false when uid is not allowed
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckOffloadPipeUidAllowed_002, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    MakeUidCheckDeviceInfoMap(configData, AUDIO_OUTPUT_FLAG_LOWPOWER, true);

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    ASSERT_NE(deviceDesc, nullptr);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    EXPECT_EQ(audioCoreService->CheckOffloadPipeUidAllowed(streamDesc, 200100), false);

    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap.erase(deviceKey);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckOffloadPipeUidAllowed_003
 * @tc.desc  : Test AudioCoreService::CheckOffloadPipeUidAllowed returns true when uid is allowed
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckOffloadPipeUidAllowed_003, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    MakeUidCheckDeviceInfoMap(configData, AUDIO_OUTPUT_FLAG_LOWPOWER, true);

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    ASSERT_NE(deviceDesc, nullptr);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    EXPECT_EQ(audioCoreService->SetOffloadAllowedForUid(200101, true), SUCCESS);

    EXPECT_EQ(audioCoreService->CheckOffloadPipeUidAllowed(streamDesc, 200101), true);

    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap.erase(deviceKey);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckOffloadPipeUidAllowed_004
 * @tc.desc  : Test AudioCoreService::CheckOffloadPipeUidAllowed returns false for AUDIO_EXT_UID
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckOffloadPipeUidAllowed_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    const int32_t AUDIO_EXT_UID = 1041;
    EXPECT_EQ(audioCoreService->CheckOffloadPipeUidAllowed(streamDesc, AUDIO_EXT_UID), false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsStreamSupportLowpower_005
 * @tc.desc  : Test AudioCoreService::IsStreamSupportLowpower returns false when uid is not allowed
 */
HWTEST_F(AudioCoreServicePrivateTest, IsStreamSupportLowpower_005, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    MakeUidCheckDeviceInfoMap(configData, AUDIO_OUTPUT_FLAG_LOWPOWER, true);

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    ASSERT_NE(deviceDesc, nullptr);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->rendererInfo_.isOffloadAllowed = true;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->callerUid_ = 200100;

    EXPECT_EQ(audioCoreService->IsStreamSupportLowpower(streamDesc), false);

    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap.erase(deviceKey);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsStreamSupportLowpower_006
 * @tc.desc  : Test AudioCoreService::IsStreamSupportLowpower returns true when uid is allowed
 */
HWTEST_F(AudioCoreServicePrivateTest, IsStreamSupportLowpower_006, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    MakeUidCheckDeviceInfoMap(configData, AUDIO_OUTPUT_FLAG_LOWPOWER, true);

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    ASSERT_NE(deviceDesc, nullptr);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->rendererInfo_.isOffloadAllowed = true;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->callerUid_ = 200101;

    EXPECT_EQ(audioCoreService->SetOffloadAllowedForUid(streamDesc->callerUid_, true), SUCCESS);
    EXPECT_EQ(audioCoreService->IsStreamSupportLowpower(streamDesc), true);

    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap.erase(deviceKey);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_123
 * @tc.desc  : Test AudioCoreService::RemoveUnusedPipe
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_123, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(pipe1, nullptr);
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER | AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD;
    pipe1->streamDescriptors_.clear();
    pipe1->moduleInfo_.className = "offload";
    audioCoreService->pipeManager_->AddAudioPipeInfo(pipe1);

    std::shared_ptr<AudioPipeInfo> pipe2 = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(pipe2, nullptr);
    pipe2->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER | AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD;
    pipe2->streamDescriptors_.clear();
    pipe2->moduleInfo_.className = "remote_offload";
    audioCoreService->pipeManager_->AddAudioPipeInfo(pipe2);

    audioCoreService->RemoveUnusedPipe();
    EXPECT_EQ(audioCoreService->pipeManager_->GetUnusedPipe().size(), 2); // 2: unused pipe size
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: LoadSplitModule_001
 * @tc.desc  : Test AudioCoreService::LoadSplitModule.
 */
HWTEST_F(AudioCoreServicePrivateTest, LoadSplitModule_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto ret = audioCoreService->LoadSplitModule("", "");
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: LoadSplitModule_002
 * @tc.desc  : Test AudioCoreService::LoadSplitModule.
 */
HWTEST_F(AudioCoreServicePrivateTest, LoadSplitModule_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::string splitArgs = "";
    std::string networkId = "b94d27b9934d3e08a52e52d7da";
    auto ret = audioCoreService->LoadSplitModule(splitArgs, networkId);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: LoadSplitModule_003
 * @tc.desc  : Test AudioCoreService::LoadSplitModule.
 */
HWTEST_F(AudioCoreServicePrivateTest, LoadSplitModule_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::string splitArgs = "8:4096:1";
    std::string networkId = "";
    auto ret = audioCoreService->LoadSplitModule(splitArgs, networkId);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: LoadSplitModule_004
 * @tc.desc  : Test AudioCoreService::LoadSplitModule.
 */
HWTEST_F(AudioCoreServicePrivateTest, LoadSplitModule_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::string splitArgs = "8:4096:1";
    std::string networkId = "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9";
    auto ret = audioCoreService->LoadSplitModule(splitArgs, networkId);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_124
 * @tc.desc  : Test AudioCoreService::OpenNewAudioPortAndRoute()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_124, TestSize.Level1)
{
    uint32_t sessionIDTest = 100;

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(pipeInfo, nullptr);
    auto audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(audioStreamDescriptor, nullptr);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    auto audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(audioStreamDescriptor, nullptr);
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioStreamDescriptor->sessionId_ = sessionIDTest;
    pipeInfo->streamDescriptors_[0]->newDeviceDescs_.push_back(audioDeviceDescriptor);

    uint32_t paIndex = 0;
    auto ret = audioCoreService->OpenNewAudioPortAndRoute(pipeInfo, paIndex);

    EXPECT_EQ(ret, sessionIDTest);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_125
 * @tc.desc  : Test AudioCoreService::OpenNewAudioPortAndRoute()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_125, TestSize.Level1)
{
    uint32_t sessionIDTest = 0;

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate =  AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(pipeInfo, nullptr);
    auto audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(audioStreamDescriptor, nullptr);
    audioStreamDescriptor->streamInfo_ = audioStreamInfo;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    pipeInfo->moduleInfo_.name = BLUETOOTH_MIC;

    auto audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(audioStreamDescriptor, nullptr);
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    pipeInfo->streamDescriptors_[0]->newDeviceDescs_.push_back(audioDeviceDescriptor);

    uint32_t paIndex = 0;
    auto ret = audioCoreService->OpenNewAudioPortAndRoute(pipeInfo, paIndex);

    EXPECT_NE(ret, sessionIDTest);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_126
 * @tc.desc  : Test AudioCoreService::OpenNewAudioPortAndRoute()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_126, TestSize.Level1)
{
    uint32_t sessionIDTest = 0;

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(pipeInfo, nullptr);

    auto audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(audioStreamDescriptor, nullptr);
    audioStreamDescriptor->routeFlag_ = AUDIO_OUTPUT_FLAG_HWDECODING;
    audioStreamDescriptor->sessionId_ = sessionIDTest;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    auto audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(audioStreamDescriptor, nullptr);
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_SPEAKER;
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    pipeInfo->streamDescriptors_[0]->newDeviceDescs_.push_back(audioDeviceDescriptor);

    uint32_t paIndex = 0;
    auto ret = audioCoreService->OpenNewAudioPortAndRoute(pipeInfo, paIndex);

    EXPECT_EQ(ret, sessionIDTest);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_127
 * @tc.desc  : Test AudioCoreService::OpenNewAudioPortAndRoute()
 */
HWTEST_F(AudioCoreServicePrivateTest, AudioCoreServicePrivate_127, TestSize.Level1)
{
    uint32_t sessionIDTest = 100;

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate =  AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(pipeInfo, nullptr);
    auto audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(audioStreamDescriptor, nullptr);
    audioStreamDescriptor->streamInfo_ = audioStreamInfo;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    pipeInfo->moduleInfo_.name = BLUETOOTH_MIC;

    auto audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(audioStreamDescriptor, nullptr);
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_SPEAKER;
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    pipeInfo->streamDescriptors_[0]->newDeviceDescs_.push_back(audioDeviceDescriptor);
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    auto curDesc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_MIC, DeviceRole::INPUT_DEVICE);
    curDesc->deviceId_ = 1001;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(curDesc);
    audioCoreService->audioRouterSelectStrategy_.UpdateCurrentInputDevice(SYSTEM_UID, {curDesc});
    uint32_t paIndex = 0;
    auto ret = audioCoreService->OpenNewAudioPortAndRoute(pipeInfo, paIndex);
    EXPECT_NE(ret, sessionIDTest);

    auto curDesc1 = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_ACCESSORY, DeviceRole::INPUT_DEVICE);
    curDesc1->deviceId_ = 1002;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(curDesc1);
    audioCoreService->audioRouterSelectStrategy_.UpdateCurrentInputDevice(SYSTEM_UID, {curDesc1});
    ret = audioCoreService->OpenNewAudioPortAndRoute(pipeInfo, paIndex);
    EXPECT_NE(ret, sessionIDTest);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsRingerOrAlarmerDualDevicesRange_001.
 * @tc.desc  : Test IsRingerOrAlarmerDualDevicesRange.
 */
HWTEST_F(AudioCoreServicePrivateTest, IsRingerOrAlarmerDualDevicesRange_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    bool ret = audioCoreService->IsRingerOrAlarmerDualDevicesRange(DEVICE_TYPE_HEARING_AID);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CaptureConcurrentCheck_001
 * @tc.desc  : Test AudioCoreService::CaptureConcurrentCheck()
 */
HWTEST_F(AudioCoreServicePrivateTest, CaptureConcurrentCheck_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest CaptureConcurrentCheck_001 start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs = {
        std::make_shared<AudioStreamDescriptor>(),
        std::make_shared<AudioStreamDescriptor>()
    };
    uint32_t flag[2] = {AUDIO_INPUT_FLAG_NORMAL, AUDIO_INPUT_FLAG_FAST};
    uint32_t originalSessionId[2] = {0};
    for (int i = 0; i < 2; i++) {
        streamDescs[i]->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
        streamDescs[i]->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
        streamDescs[i]->streamInfo_.channels = AudioChannel::STEREO;
        streamDescs[i]->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
        streamDescs[i]->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
        streamDescs[i]->rendererInfo_.streamUsage = STREAM_USAGE_MOVIE;

        streamDescs[i]->audioMode_ = AUDIO_MODE_RECORD;
        streamDescs[i]->createTimeStamp_ = ClockTime::GetCurNano();
        streamDescs[i]->stateStartTimeStamp_ = streamDescs[i]->createTimeStamp_ + 1;
        streamDescs[i]->callerUid_ = getuid();
        auto result = audioCoreService->CreateCapturerClient(streamDescs[i], flag[i], originalSessionId[i]);
        EXPECT_EQ(result, SUCCESS);
    }
    auto dfxResult = std::make_unique<struct ConcurrentCaptureDfxResult>();
    audioCoreService->WriteCapturerConcurrentMsg(streamDescs[0], dfxResult);
    audioCoreService->LogCapturerConcurrentResult(dfxResult);
    audioCoreService->WriteCapturerConcurrentEvent(dfxResult);
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest CaptureConcurrentCheck_001 end");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CaptureConcurrentCheck_002
 * @tc.desc  : Test AudioCoreService::CaptureConcurrentCheck()
 */
HWTEST_F(AudioCoreServicePrivateTest, CaptureConcurrentCheck_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest CaptureConcurrentCheck_002 start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    auto dfxResult = std::make_unique<struct ConcurrentCaptureDfxResult>();
    for (int i = 0; i < 5; i++) {
        dfxResult->existingAppName.push_back("www.test.com");
        dfxResult->existingAppState.push_back(static_cast<uint8_t>(2));
        dfxResult->existingSourceType.push_back(static_cast<uint8_t>(SourceType::SOURCE_TYPE_MIC));
        dfxResult->existingCaptureState.push_back(static_cast<uint8_t>(2));
        dfxResult->existingCreateDuration.push_back(static_cast<uint32_t>(0));
        dfxResult->existingStartDuration.push_back(static_cast<uint32_t>(i));
        dfxResult->existingFastFlag.push_back(static_cast<bool>(0));
    }
    dfxResult->hdiSourceType = 1;
    dfxResult->hdiSourceAlg = "develope test";
    dfxResult->deviceType = DEVICE_TYPE_MIC;
    audioCoreService->LogCapturerConcurrentResult(dfxResult);
    audioCoreService->WriteCapturerConcurrentEvent(dfxResult);
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest CaptureConcurrentCheck_002 end");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: ActivateInputDevice_001
 * @tc.desc  : Test AudioCoreService::ActivateInputDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, ActivateInputDevice_001, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest ActivateInputDevice_001 start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(streamDesc, nullptr);

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_ARM_HEADSET,
        DeviceRole::INPUT_DEVICE);
    EXPECT_NE(deviceDesc, nullptr);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    auto mockDeviceBehavior = std::make_shared<MockAudioArmDeviceBehavior>();
    ASSERT_NE(mockDeviceBehavior, nullptr);

    EXPECT_CALL(*mockDeviceBehavior, ActivateDevice(_, _)).WillOnce(Return(SUCCESS));

    auto& audioDeviceFactory = AudioDeviceFactory::GetInstance();
    audioDeviceFactory.deviceBehaviorCache_[DEVICE_TYPE_USB_ARM_HEADSET] = mockDeviceBehavior;

    auto result = audioCoreService->ActivateInputDevice(streamDesc);
    ASSERT_EQ(result, SUCCESS);
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest ActivateInputDevice_001 end");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: ActivateInputDevice_002
 * @tc.desc  : Test AudioCoreService::ActivateInputDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, ActivateInputDevice_002, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest ActivateInputDevice_002 start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(streamDesc, nullptr);

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_NONE,
        DeviceRole::INPUT_DEVICE);
    EXPECT_NE(deviceDesc, nullptr);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    auto result = audioCoreService->ActivateInputDevice(streamDesc);
    EXPECT_EQ(result, SUCCESS);
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest ActivateInputDevice_002 end");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: MuteSinkPortForSwitchDevice_001
 * @tc.desc  : Test AudioCoreService::MuteSinkPortForSwitchDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, MuteSinkPortForSwitchDevice_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    // Test1
    AudioStreamDeviceChangeReasonExt::ExtEnum extReason = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extReason);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    streamDesc->oldDeviceDescs_.push_back(oldDesc);
    streamDesc->newDeviceDescs_.push_back(newDesc);
    audioCoreService->audioIOHandleMap_.SetMoveFinish(true);

    audioCoreService->MuteSinkPortForSwitchDevice(streamDesc, reason);
    EXPECT_TRUE(audioCoreService->audioIOHandleMap_.moveDeviceFinished_.load());
    streamDesc->newDeviceDescs_.clear();

    // Test2
    newDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(newDesc);

    audioCoreService->MuteSinkPortForSwitchDevice(streamDesc, reason);
    EXPECT_FALSE(audioCoreService->audioIOHandleMap_.moveDeviceFinished_.load());

    // Test3
    audioCoreService->audioIOHandleMap_.SetMoveFinish(true);
    streamDesc->oldRouteFlag_ = (AUDIO_OUTPUT_FLAG_FAST | AUDIO_OUTPUT_FLAG_VOIP);
    streamDesc->routeFlag_ = (AUDIO_OUTPUT_FLAG_DIRECT | AUDIO_OUTPUT_FLAG_VOIP);

    audioCoreService->MuteSinkPortForSwitchDevice(streamDesc, reason);
    EXPECT_FALSE(audioCoreService->audioIOHandleMap_.moveDeviceFinished_.load());
    audioCoreService->audioIOHandleMap_.SetMoveFinish(false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: MuteSinkPortForSwitchDevice_002
 * @tc.desc  : Test AudioCoreService::MuteSinkPortForSwitchDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, MuteSinkPortForSwitchDevice_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    // Test4
    AudioStreamDeviceChangeReasonExt::ExtEnum extReason = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extReason);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    streamDesc->oldDeviceDescs_.push_back(oldDesc);
    streamDesc->newDeviceDescs_.push_back(newDesc);
    streamDesc->oldRouteFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    audioCoreService->audioIOHandleMap_.SetMoveFinish(true);

    audioCoreService->MuteSinkPortForSwitchDevice(streamDesc, reason);
    EXPECT_FALSE(audioCoreService->audioIOHandleMap_.moveDeviceFinished_.load());
    streamDesc->newDeviceDescs_.clear();


    // Test5
    newDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_ARM_HEADSET, DeviceRole::OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(newDesc);

    audioCoreService->MuteSinkPortForSwitchDevice(streamDesc, reason);
    EXPECT_FALSE(audioCoreService->audioIOHandleMap_.moveDeviceFinished_.load());
    audioCoreService->audioIOHandleMap_.SetMoveFinish(false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: MuteSinkPortForSwitchDevice_003
 * @tc.desc  : Test AudioCoreService::MuteSinkPortForSwitchDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, MuteSinkPortForSwitchDevice_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    // Test5
    AudioStreamDeviceChangeReasonExt::ExtEnum extReason = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extReason);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_HEADSET, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->oldDeviceDescs_.push_back(oldDesc);
    streamDesc->newDeviceDescs_.push_back(newDesc);
    streamDesc->oldRouteFlag_ = (AUDIO_OUTPUT_FLAG_DIRECT | AUDIO_OUTPUT_FLAG_HD);
    audioCoreService->audioIOHandleMap_.SetMoveFinish(true);

    audioCoreService->MuteSinkPortForSwitchDevice(streamDesc, reason);
    EXPECT_FALSE(audioCoreService->audioIOHandleMap_.moveDeviceFinished_.load());
    audioCoreService->audioIOHandleMap_.SetMoveFinish(false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckAndSleepBeforeVoiceCallDeviceSet_001
 * @tc.desc  : Test AudioCoreService::CheckAndSleepBeforeVoiceCallDeviceSet()
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndSleepBeforeVoiceCallDeviceSet_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    // Test1
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeVoiceCallDeviceSet(reason);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 3000; // 3ms
    EXPECT_LE(duration, deltaUs);

    // Test2
    reason = AudioStreamDeviceChangeReasonExt::ExtEnum::SET_AUDIO_SCENE;
    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    info->rendererState = RENDERER_RUNNING;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeVoiceCallDeviceSet(reason);
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    EXPECT_LE(duration, deltaUs);

    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();

    // Test3
    info->rendererInfo.streamUsage = STREAM_USAGE_VOICE_RINGTONE;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeVoiceCallDeviceSet(reason);
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t targetTime = 120000; //120ms
    EXPECT_GE(duration, targetTime);

    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: HandlePrimaryMediaMuteForDualRing_001
 * @tc.desc  : Execute mute when all conditions are met
 */
HWTEST_F(AudioCoreServicePrivateTest, HandlePrimaryMediaMuteForDualRing_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> newDesc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_HEADSET, DeviceRole::OUTPUT_DEVICE);
    newDesc1->networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioDeviceDescriptor> newDesc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    newDesc2->networkId_ = LOCAL_NETWORK_ID;
    streamDesc->newDeviceDescs_.push_back(newDesc1);
    streamDesc->newDeviceDescs_.push_back(newDesc2);

    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    info->rendererState = RENDERER_RUNNING;
    info->sessionId = 10000;
    info->outputDeviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    info->outputDeviceInfo.networkId_ = LOCAL_NETWORK_ID;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    std::shared_ptr<AudioStreamDescriptor> mediaStreamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_HEADSET, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = LOCAL_NETWORK_ID;
    mediaStreamDesc->newDeviceDescs_.push_back(desc);
    mediaStreamDesc->sessionId_ = 10000;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->AddStream(mediaStreamDesc);
    audioCoreService->pipeManager_->AddAudioPipeInfo(pipeInfo);

    audioCoreService->HandlePrimaryMediaMuteForDualRing(streamDesc);
    EXPECT_EQ(1, audioCoreService->streamsWhenRingDualOnPrimarySpeaker_.size());
    streamDesc->newDeviceDescs_.clear();
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
    audioCoreService->streamsWhenRingDualOnPrimarySpeaker_.clear();
    audioCoreService->pipeManager_->RemoveAudioPipeInfo(pipeInfo);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: HandlePrimaryMediaMuteForDualRing_002
 * @tc.desc  : Do nothing when device is neither ring nor alarm device
 */
HWTEST_F(AudioCoreServicePrivateTest, HandlePrimaryMediaMuteForDualRing_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> newDesc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    newDesc1->networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioDeviceDescriptor> newDesc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    newDesc2->networkId_ = LOCAL_NETWORK_ID;
    streamDesc->newDeviceDescs_.push_back(newDesc1);
    streamDesc->newDeviceDescs_.push_back(newDesc2);

    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    info->rendererState = RENDERER_RUNNING;
    info->sessionId = 10000;
    info->outputDeviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    info->outputDeviceInfo.networkId_ = LOCAL_NETWORK_ID;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    audioCoreService->HandlePrimaryMediaMuteForDualRing(streamDesc);
    EXPECT_EQ(0, audioCoreService->streamsWhenRingDualOnPrimarySpeaker_.size());
    streamDesc->newDeviceDescs_.clear();
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
    audioCoreService->streamsWhenRingDualOnPrimarySpeaker_.clear();
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: HandlePrimaryMediaMuteForDualRing_003
 * @tc.desc  : Do nothing when ringtone is not double-played on the primary output
 */
HWTEST_F(AudioCoreServicePrivateTest, HandlePrimaryMediaMuteForDualRing_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> newDesc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    newDesc1->networkId_ = LOCAL_NETWORK_ID;
    streamDesc->newDeviceDescs_.push_back(newDesc1);

    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    info->rendererState = RENDERER_RUNNING;
    info->sessionId = 10000;
    info->outputDeviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    info->outputDeviceInfo.networkId_ = LOCAL_NETWORK_ID;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    audioCoreService->HandlePrimaryMediaMuteForDualRing(streamDesc);
    EXPECT_EQ(0, audioCoreService->streamsWhenRingDualOnPrimarySpeaker_.size());
    streamDesc->newDeviceDescs_.clear();
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
    audioCoreService->streamsWhenRingDualOnPrimarySpeaker_.clear();
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckAndSleepBeforeRingDualDeviceSet_001
 * @tc.desc  : Test AudioCoreService::CheckAndSleepBeforeRingDualDeviceSet()
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndSleepBeforeRingDualDeviceSet_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    // Test1
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ == STREAM_STATUS_NEW;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    std::shared_ptr<AudioDeviceDescriptor> newDesc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_HEADSET, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(newDesc1);
    streamDesc->newDeviceDescs_.push_back(newDesc2);

    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    info->rendererState = RENDERER_RUNNING;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t targetTime = 120000; //120ms
    EXPECT_GE(duration, targetTime);
    streamDesc->newDeviceDescs_.clear();
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();

    // Test2
    std::shared_ptr<AudioDeviceDescriptor> newDesc3 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(newDesc3);

    start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc);
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 5000; // 5ms
    EXPECT_LE(duration, deltaUs);
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckAndSleepBeforeRingDualDeviceSet_002
 * @tc.desc  : Test AudioCoreService::CheckAndSleepBeforeRingDualDeviceSet()
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndSleepBeforeRingDualDeviceSet_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    // Test3
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ == STREAM_STATUS_NEW;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    info->rendererState = RENDERER_RUNNING;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 5000; // 5ms
    EXPECT_LE(duration, deltaUs);
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckAndSleepBeforeRingDualDeviceSet_003
 * @tc.desc  : Test AudioCoreService::CheckAndSleepBeforeRingDualDeviceSet()
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndSleepBeforeRingDualDeviceSet_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    // Test4
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ == STREAM_STATUS_NEW;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MOVIE;

    std::shared_ptr<AudioDeviceDescriptor> newDesc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_HEADSET, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(newDesc1);
    streamDesc->newDeviceDescs_.push_back(newDesc2);

    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    info->rendererState = RENDERER_RUNNING;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 5000; // 5ms
    EXPECT_LE(duration, deltaUs);
    streamDesc->newDeviceDescs_.clear();
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();

    // Test5
    streamDesc->streamStatus_ == STREAM_STATUS_NEW;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_ALARM;

    info->rendererState = RENDERER_STOPPED;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc);
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    EXPECT_LE(duration, deltaUs);
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckAndSleepBeforeRingDualDeviceSet_004
 * @tc.desc  : Test AudioCoreService::CheckAndSleepBeforeRingDualDeviceSet()
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndSleepBeforeRingDualDeviceSet_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    // Test6
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ == STREAM_STATUS_NEW;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_ALARM;

    std::shared_ptr<AudioDeviceDescriptor> newDesc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(newDesc1);
    streamDesc->newDeviceDescs_.push_back(newDesc2);

    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    info->rendererState = RENDERER_RUNNING;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t targetTime = 120000; //120ms
    EXPECT_GE(duration, targetTime);
    streamDesc->newDeviceDescs_.clear();
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckAndSleepBeforeRingDualDeviceSet_005
 * @tc.desc  : Test AudioCoreService::CheckAndSleepBeforeRingDualDeviceSet()
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndSleepBeforeRingDualDeviceSet_005, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    // Test7
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ == STREAM_STATUS_NEW;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    std::shared_ptr<AudioDeviceDescriptor> newDesc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_HEADSET, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(newDesc1);
    streamDesc->newDeviceDescs_.push_back(newDesc2);

    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    info->rendererState = RENDERER_RUNNING;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t targetTime = 120000; //120ms
    EXPECT_GE(duration, targetTime);
    streamDesc->newDeviceDescs_.clear();
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: SleepForSwitchDevice_001
 * @tc.desc  : Test AudioCoreService::SleepForSwitchDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, SleepForSwitchDevice_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    streamDesc->oldDeviceDescs_.push_back(oldDesc);
    streamDesc->newDeviceDescs_.push_back(newDesc);

    AudioStreamDeviceChangeReasonExt::ExtEnum extReason = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extReason);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->SleepForSwitchDevice(streamDesc, reason);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 5000; // 5ms
    EXPECT_LE(duration, deltaUs);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: SleepForSwitchDevice_002
 * @tc.desc  : Test AudioCoreService::SleepForSwitchDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, SleepForSwitchDevice_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->oldDeviceDescs_.push_back(oldDesc);
    streamDesc->newDeviceDescs_.push_back(newDesc);

    AudioStreamDeviceChangeReasonExt::ExtEnum extReason = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extReason);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->SleepForSwitchDevice(streamDesc, reason);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 5000; // 5ms
    uint32_t targetTime = 160000; // 160ms
    EXPECT_GE(duration, targetTime);
    EXPECT_LE(duration, targetTime + deltaUs);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: SleepForSwitchDevice_003
 * @tc.desc  : Test AudioCoreService::SleepForSwitchDevice()
 */
HWTEST_F(AudioCoreServicePrivateTest, SleepForSwitchDevice_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->oldDeviceDescs_.push_back(oldDesc);
    streamDesc->newDeviceDescs_.push_back(newDesc);

    AudioStreamDeviceChangeReasonExt::ExtEnum extReason =
        AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE;
    AudioStreamDeviceChangeReasonExt reason(extReason);

    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;

    auto start = std::chrono::steady_clock::now();
    audioCoreService->SleepForSwitchDevice(streamDesc, reason);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 5000; // 5ms
    EXPECT_LE(duration, deltaUs);

    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AddSessionId_001
 * @tc.desc  : Test AudioCoreService::AddSessionId()
 */
HWTEST_F(AudioCoreServicePrivateTest, AddSessionId_001, TestSize.Level3)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    uint32_t sessionId = 1;
    audioCoreService->AddSessionId(sessionId);
    EXPECT_EQ(1, audioCoreService->sessionIdMap_.count(sessionId));
    audioCoreService->DeleteSessionId(sessionId);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckAndUpdateHearingAidCall_001
 * @tc.desc  : Test AudioCoreService::CheckAndUpdateHearingAidCall
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndUpdateHearingAidCall_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    audioCoreService->hearingAidCallFlag_ = false;
    audioCoreService->CheckAndUpdateHearingAidCall(DeviceType::DEVICE_TYPE_HEARING_AID);
    audioCoreService->hearingAidCallFlag_ = true;
    audioCoreService->CheckAndUpdateHearingAidCall(DeviceType::DEVICE_TYPE_EARPIECE);
    ASSERT_EQ(audioCoreService->hearingAidCallFlag_, false);

    audioCoreService->CheckAndUpdateHearingAidCall(DeviceType::DEVICE_TYPE_HEARING_AID);
    auto ret = AudioPipeManager::GetPipeManager()->GetPipeinfoByNameAndFlag("hearing_aid",
        AUDIO_OUTPUT_FLAG_NORMAL);
    ASSERT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: ActivateOutputDevice_001
 * @tc.desc  : Test AudioCoreService::ActivateOutputDevice
 */
HWTEST_F(AudioCoreServicePrivateTest, ActivateOutputDevice_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    newDeviceDesc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    newDeviceDesc->deviceId_ = 1;
    newDeviceDesc->deviceName_ = "usb_arm_headset";

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);

    auto mockDeviceBehavior = std::make_shared<MockAudioArmDeviceBehavior>();
    ASSERT_NE(mockDeviceBehavior, nullptr);

    EXPECT_CALL(*mockDeviceBehavior, ActivateDevice(_, _)).WillOnce(Return(SUCCESS));

    auto& audioDeviceFactory = AudioDeviceFactory::GetInstance();
    audioDeviceFactory.deviceBehaviorCache_[DEVICE_TYPE_USB_ARM_HEADSET] = mockDeviceBehavior;

    int32_t ret = audioCoreService->ActivateOutputDevice(streamDesc);
    ASSERT_NE(audioCoreService, nullptr);
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_IsDupDeviceChange_001
 * @tc.number : IsDupDeviceChange_001
 * @tc.desc   : Test IsDupDeviceChange() for nullptr
 */
HWTEST_F(AudioCoreServicePrivateTest, IsDupDeviceChange_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    bool isDeviceChange = audioCoreService->IsDupDeviceChange(nullptr);

    EXPECT_EQ(isDeviceChange, false);
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_IsDupDeviceChange_002
 * @tc.number : IsDupDeviceChange_002
 * @tc.desc   : Test IsDupDeviceChange() for diff array size
 */
HWTEST_F(AudioCoreServicePrivateTest, IsDupDeviceChange_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    newDeviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDupDeviceDescs_.push_back(newDeviceDesc);

    bool isDeviceChange = audioCoreService->IsDupDeviceChange(streamDesc);

    EXPECT_EQ(isDeviceChange, true);
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_IsDupDeviceChange_003
 * @tc.number : IsDupDeviceChange_003
 * @tc.desc   : Test IsDupDeviceChange() for same device
 */
HWTEST_F(AudioCoreServicePrivateTest, IsDupDeviceChange_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    newDeviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDupDeviceDescs_.push_back(newDeviceDesc);

    std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    oldDeviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->oldDupDeviceDescs_.push_back(oldDeviceDesc);

    bool isDeviceChange = audioCoreService->IsDupDeviceChange(streamDesc);

    EXPECT_EQ(isDeviceChange, false);
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_IsDupDeviceChange_004
 * @tc.number : IsDupDeviceChange_004
 * @tc.desc   : Test IsDupDeviceChange() for diff device
 */
HWTEST_F(AudioCoreServicePrivateTest, IsDupDeviceChange_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    newDeviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDupDeviceDescs_.push_back(newDeviceDesc);

    std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    oldDeviceDesc->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    streamDesc->oldDupDeviceDescs_.push_back(oldDeviceDesc);

    bool isDeviceChange = audioCoreService->IsDupDeviceChange(streamDesc);

    EXPECT_EQ(isDeviceChange, true);
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_IsDupDeviceChange_005
 * @tc.number : IsDupDeviceChange_005
 * @tc.desc   : Test IsDupDeviceChange() for no dup device
 */
HWTEST_F(AudioCoreServicePrivateTest, IsDupDeviceChange_005, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->oldDupDeviceDescs_.clear();
    streamDesc->newDupDeviceDescs_.clear();

    bool isDeviceChange = audioCoreService->IsDupDeviceChange(streamDesc);

    EXPECT_EQ(isDeviceChange, false);
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_CheckAndUpdateOffloadEnableForStream_001
 * @tc.number : CheckAndUpdateOffloadEnableForStream_001
 * @tc.desc   : Test CheckAndUpdateOffloadEnableForStream() for offload new case
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndUpdateOffloadEnableForStream_001, TestSize.Level3)
{
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = TEST_STREAM_1_SESSION_ID;
    stream->SetRoute(AUDIO_OUTPUT_FLAG_LOWPOWER);
    testCoreService_->CheckAndUpdateOffloadEnableForStream(OFFLOAD_NEW, stream, TEST_IO_HANDLE);
    EXPECT_EQ(TEST_STREAM_1_SESSION_ID, testCoreService_->audioOffloadStream_.GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_CheckAndUpdateOffloadEnableForStream_002
 * @tc.number : CheckAndUpdateOffloadEnableForStream_002
 * @tc.desc   : Test CheckAndUpdateOffloadEnableForStream() for offload move in case
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndUpdateOffloadEnableForStream_002, TestSize.Level3)
{
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = TEST_STREAM_1_SESSION_ID;
    stream->SetRoute(AUDIO_OUTPUT_FLAG_LOWPOWER);
    testCoreService_->CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_IN, stream, TEST_IO_HANDLE);
    EXPECT_EQ(TEST_STREAM_1_SESSION_ID, testCoreService_->audioOffloadStream_.GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_CheckAndUpdateOffloadEnableForStream_003
 * @tc.number : CheckAndUpdateOffloadEnableForStream_003
 * @tc.desc   : Test CheckAndUpdateOffloadEnableForStream() for offload move in case
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndUpdateOffloadEnableForStream_003, TestSize.Level4)
{
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = TEST_STREAM_2_SESSION_ID;
    stream->SetRoute(AUDIO_OUTPUT_FLAG_NORMAL);
    testCoreService_->CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_IN, stream, TEST_IO_HANDLE);
    EXPECT_NE(TEST_STREAM_2_SESSION_ID, testCoreService_->audioOffloadStream_.GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_CheckAndUpdateOffloadEnableForStream_004
 * @tc.number : CheckAndUpdateOffloadEnableForStream_004
 * @tc.desc   : Test CheckAndUpdateOffloadEnableForStream() for offload move out case
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndUpdateOffloadEnableForStream_004, TestSize.Level3)
{
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = TEST_STREAM_1_SESSION_ID;
    stream->SetRoute(AUDIO_OUTPUT_FLAG_NORMAL);
    stream->SetOldRoute(AUDIO_OUTPUT_FLAG_LOWPOWER);
    testCoreService_->CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_OUT, stream, TEST_IO_HANDLE);
    EXPECT_NE(TEST_STREAM_1_SESSION_ID, testCoreService_->audioOffloadStream_.GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_CheckAndUpdateOffloadEnableForStream_005
 * @tc.number : CheckAndUpdateOffloadEnableForStream_005
 * @tc.desc   : Test CheckAndUpdateOffloadEnableForStream() for offload move out case
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndUpdateOffloadEnableForStream_005, TestSize.Level3)
{
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = TEST_STREAM_1_SESSION_ID;
    stream->SetRoute(AUDIO_OUTPUT_FLAG_NORMAL);
    stream->SetOldRoute(AUDIO_OUTPUT_FLAG_NORMAL);
    testCoreService_->CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_OUT, stream, TEST_IO_HANDLE);
    EXPECT_NE(TEST_STREAM_1_SESSION_ID, testCoreService_->audioOffloadStream_.GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_CheckAndUpdateOffloadEnableForStream_006
 * @tc.number : CheckAndUpdateOffloadEnableForStream_006
 * @tc.desc   : Test CheckAndUpdateOffloadEnableForStream() for offload move out case
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndUpdateOffloadEnableForStream_006, TestSize.Level3)
{
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = TEST_STREAM_1_SESSION_ID;
    stream->SetRoute(AUDIO_OUTPUT_FLAG_LOWPOWER);
    stream->SetOldRoute(AUDIO_OUTPUT_FLAG_NORMAL);
    testCoreService_->CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_OUT, stream, TEST_IO_HANDLE);
    EXPECT_NE(TEST_STREAM_1_SESSION_ID, testCoreService_->audioOffloadStream_.GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_CheckAndUpdateOffloadEnableForStream_007
 * @tc.number : CheckAndUpdateOffloadEnableForStream_007
 * @tc.desc   : Test CheckAndUpdateOffloadEnableForStream() for offload move out case
 */
HWTEST_F(AudioCoreServicePrivateTest, CheckAndUpdateOffloadEnableForStream_007, TestSize.Level3)
{
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = TEST_STREAM_1_SESSION_ID;
    stream->SetRoute(AUDIO_OUTPUT_FLAG_LOWPOWER);
    stream->SetOldRoute(AUDIO_OUTPUT_FLAG_LOWPOWER);
    testCoreService_->CheckAndUpdateOffloadEnableForStream(OFFLOAD_MOVE_OUT, stream, TEST_IO_HANDLE);
    EXPECT_NE(TEST_STREAM_1_SESSION_ID, testCoreService_->audioOffloadStream_.GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_DeactivateBluetoothDevice_001
 * @tc.number : DeactivateBluetoothDevice_001
 * @tc.desc   : Test DeactivateBluetoothDevice for empty A2dpdevice MAC address and empty hfpdevice MAC address
 */
HWTEST_F(AudioCoreServicePrivateTest, DeactivateBluetoothDevice_001, TestSize.Level3)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    bool isRunning = true;
    audioCoreService->DeactivateBluetoothDevice(isRunning);
    EXPECT_EQ(Bluetooth::AudioA2dpManager::GetActiveA2dpDeviceLocal(), EMPTY_ADDRESS);
    EXPECT_EQ(Bluetooth::AudioHfpManager::GetActiveHfpDeviceLocal(), EMPTY_ADDRESS);
}

/**
 * @tc.name  : AudioCoreServicePrivateTest_FetchRendererPipeAndExecute_001
 * @tc.number: FetchRendererPipeAndExecute_001
 * @tc.desc  : Test AudioCoreService::FetchRendererPipeAndExecute()
 */
HWTEST_F(AudioCoreServicePrivateTest, FetchRendererPipeAndExecute_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    std::shared_ptr<AdapterPipeInfo> adapterPipeInfo = std::make_shared<AdapterPipeInfo>();
    std::shared_ptr<PolicyAdapterInfo> policyAdapterInfo = std::make_shared<PolicyAdapterInfo>();
    MakeDeviceInfoMap(configData, adapterPipeInfo, policyAdapterInfo);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->deviceRole_ = OUTPUT_DEVICE;
    streamDesc->newDeviceDescs_.front()->networkId_ = "test_networkId";
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::CHANNEL_6;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_5POINT1;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList;
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo->adapterName_ = "remote";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    pipeInfo->name_ = "offload_distributed_output";
    pipeInfo->moduleInfo_.format = AudioDefinitionPolicyUtils::enumToFormatStr[AudioSampleFormat::SAMPLE_S16LE];
    pipeInfo->moduleInfo_.rate = std::to_string(AudioSamplingRate::SAMPLE_RATE_48000);
    pipeInfo->moduleInfo_.channels = std::to_string(ConvertLayoutToAudioChannel(AudioChannelLayout::CH_LAYOUT_STEREO));
    pipeInfo->moduleInfo_.channelLayout = std::to_string(AudioChannelLayout::CH_LAYOUT_STEREO);
    pipeInfo->moduleInfo_.networkId = "test_networkId";
    pipeInfoList.push_back(pipeInfo);
    AudioPipeManager::GetPipeManager()->curPipeList_ = pipeInfoList;

    uint32_t sessionId = 100001;
    uint32_t audioFlag = 16;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->FetchRendererPipeAndExecute(streamDesc, sessionId, audioFlag, reason);
    EXPECT_TRUE(AudioPipeManager::GetPipeManager()->curPipeList_.size() == 1);

    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap.erase(deviceKey);
    AudioPolicyConfigData::GetInstance().ClearDynamicStreamProps("remote", "offload_distributed_output");
}

/**
 * @tc.name  : AudioCoreServicePrivateTest_FetchRendererPipeAndExecute_002
 * @tc.number: FetchRendererPipeAndExecute_002
 * @tc.desc  : Test AudioCoreService::FetchRendererPipeAndExecute()
 */
HWTEST_F(AudioCoreServicePrivateTest, FetchRendererPipeAndExecute_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    std::shared_ptr<AdapterPipeInfo> adapterPipeInfo = std::make_shared<AdapterPipeInfo>();
    std::shared_ptr<PolicyAdapterInfo> policyAdapterInfo = std::make_shared<PolicyAdapterInfo>();
    MakeDeviceInfoMap(configData, adapterPipeInfo, policyAdapterInfo);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->deviceRole_ = OUTPUT_DEVICE;
    streamDesc->newDeviceDescs_.front()->networkId_ = "test_networkId";
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList;
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo->adapterName_ = "primary";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    pipeInfo->moduleInfo_.networkId = "test_networkId";
    pipeInfoList.push_back(pipeInfo);
    AudioPipeManager::GetPipeManager()->curPipeList_ = pipeInfoList;

    uint32_t sessionId = 100001;
    uint32_t audioFlag = 16;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->FetchRendererPipeAndExecute(streamDesc, sessionId, audioFlag, reason);
    EXPECT_TRUE(AudioPipeManager::GetPipeManager()->curPipeList_.size() == 1);

    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap.erase(deviceKey);
    AudioPolicyConfigData::GetInstance().ClearDynamicStreamProps("remote", "offload_distributed_output");
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_HandleRingToNonRingSceneChange_001
 * @tc.number : HandleRingToNonRingSceneChange_001
 * @tc.desc   : Test HandleRingToNonRingSceneChange() for audioscene change.
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleRingToNonRingSceneChange_001, TestSize.Level3)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    audioCoreService->isRingDualToneOnPrimarySpeaker_ = true;
    audioCoreService->HandleRingToNonRingSceneChange(AUDIO_SCENE_VOICE_RINGING, AUDIO_SCENE_DEFAULT);
    EXPECT_FALSE(audioCoreService->isRingDualToneOnPrimarySpeaker_);
    audioCoreService->HandleRingToNonRingSceneChange(AUDIO_SCENE_VOICE_RINGING, AUDIO_SCENE_PHONE_CALL);
    EXPECT_FALSE(audioCoreService->isRingDualToneOnPrimarySpeaker_);
    audioCoreService->HandleRingToNonRingSceneChange(AUDIO_SCENE_VOICE_RINGING, AUDIO_SCENE_PHONE_CHAT);
    EXPECT_FALSE(audioCoreService->isRingDualToneOnPrimarySpeaker_);

    audioCoreService->isRingDualToneOnPrimarySpeaker_ = true;
    audioCoreService->HandleRingToNonRingSceneChange(AUDIO_SCENE_RINGING, AUDIO_SCENE_DEFAULT);
    EXPECT_FALSE(audioCoreService->isRingDualToneOnPrimarySpeaker_);
    audioCoreService->HandleRingToNonRingSceneChange(AUDIO_SCENE_RINGING, AUDIO_SCENE_PHONE_CALL);
    EXPECT_FALSE(audioCoreService->isRingDualToneOnPrimarySpeaker_);
    audioCoreService->HandleRingToNonRingSceneChange(AUDIO_SCENE_RINGING, AUDIO_SCENE_PHONE_CHAT);
    EXPECT_FALSE(audioCoreService->isRingDualToneOnPrimarySpeaker_);

    audioCoreService->isRingDualToneOnPrimarySpeaker_ = true;
    audioCoreService->HandleRingToNonRingSceneChange(AUDIO_SCENE_DEFAULT, AUDIO_SCENE_DEFAULT);
    EXPECT_TRUE(audioCoreService->isRingDualToneOnPrimarySpeaker_);
    audioCoreService->HandleRingToNonRingSceneChange(AUDIO_SCENE_DEFAULT, AUDIO_SCENE_PHONE_CALL);
    EXPECT_TRUE(audioCoreService->isRingDualToneOnPrimarySpeaker_);
    audioCoreService->HandleRingToNonRingSceneChange(AUDIO_SCENE_DEFAULT, AUDIO_SCENE_PHONE_CHAT);
    EXPECT_TRUE(audioCoreService->isRingDualToneOnPrimarySpeaker_);
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenFetch
 * @tc.number : HandleA2dpSuspendWhenFetch_001
 * @tc.desc   : Test HandleA2dpSuspendWhenFetch when other reason
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspendWhenFetch_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioStreamDeviceChangeReasonExt reason { AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN };
    AudioDeviceDescriptor actived;
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    EXPECT_FALSE(audioCoreService->HandleA2dpSuspendWhenFetch(reason, actived, streams));
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenFetch
 * @tc.number : HandleA2dpSuspendWhenFetch_002
 * @tc.desc   : Test HandleA2dpSuspendWhenFetch when old device unavailable and empty streams
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspendWhenFetch_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioStreamDeviceChangeReasonExt reason { AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE };
    AudioDeviceDescriptor actived;
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    EXPECT_FALSE(audioCoreService->HandleA2dpSuspendWhenFetch(reason, actived, streams));
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenFetch
 * @tc.number : HandleA2dpSuspendWhenFetch_003
 * @tc.desc   : Test HandleA2dpSuspendWhenFetch when old device unavailable and stream is nullptr
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspendWhenFetch_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioStreamDeviceChangeReasonExt reason { AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE };
    AudioDeviceDescriptor actived { DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE };
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    std::shared_ptr<AudioStreamDescriptor> stream { nullptr };
    streams.emplace_back(stream);
    EXPECT_FALSE(audioCoreService->HandleA2dpSuspendWhenFetch(reason, actived, streams));
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenFetch
 * @tc.number : HandleA2dpSuspendWhenFetch_004
 * @tc.desc   : Test HandleA2dpSuspendWhenFetch when old device unavailable and stream is STREAM_STATUS_NEW
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspendWhenFetch_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioStreamDeviceChangeReasonExt reason { AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE };
    AudioDeviceDescriptor actived { DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE };
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->streamStatus_ = STREAM_STATUS_NEW;
    stream->newDeviceDescs_.emplace_back(std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE));
    streams.emplace_back(stream);
    EXPECT_FALSE(audioCoreService->HandleA2dpSuspendWhenFetch(reason, actived, streams));
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenFetch
 * @tc.number : HandleA2dpSuspendWhenFetch_005
 * @tc.desc   : Test HandleA2dpSuspendWhenFetch when old device unavailable and stream new devices is empty
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspendWhenFetch_005, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioStreamDeviceChangeReasonExt reason { AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE };
    AudioDeviceDescriptor actived { DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE };
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->streamStatus_ = STREAM_STATUS_STARTED;
    stream->newDeviceDescs_.clear();
    streams.emplace_back(stream);
    EXPECT_FALSE(audioCoreService->HandleA2dpSuspendWhenFetch(reason, actived, streams));
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenFetch
 * @tc.number : HandleA2dpSuspendWhenFetch_006
 * @tc.desc   : Test HandleA2dpSuspendWhenFetch when old device unavailable and actived will not change
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspendWhenFetch_006, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioStreamDeviceChangeReasonExt reason { AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE };
    AudioDeviceDescriptor actived { DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE };
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->streamStatus_ = STREAM_STATUS_STARTED;
    stream->newDeviceDescs_.emplace_back(std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE));
    streams.emplace_back(stream);
    EXPECT_FALSE(audioCoreService->HandleA2dpSuspendWhenFetch(reason, actived, streams));
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenFetch
 * @tc.number : HandleA2dpSuspendWhenFetch_007
 * @tc.desc   : Test HandleA2dpSuspendWhenFetch when old device unavailable and actived all will not change
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspendWhenFetch_007, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioStreamDeviceChangeReasonExt reason { AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE };
    AudioDeviceDescriptor actived { DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE };
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    auto streamA = std::make_shared<AudioStreamDescriptor>();
    streamA->streamStatus_ = STREAM_STATUS_STARTED;
    streamA->newDeviceDescs_.emplace_back(std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE));
    streams.emplace_back(streamA);
    auto streamB = std::make_shared<AudioStreamDescriptor>();
    streamB->streamStatus_ = STREAM_STATUS_STARTED;
    streamB->newDeviceDescs_.emplace_back(std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE));
    streams.emplace_back(streamB);
    EXPECT_FALSE(audioCoreService->HandleA2dpSuspendWhenFetch(reason, actived, streams));
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenFetch
 * @tc.number : HandleA2dpSuspendWhenFetch_008
 * @tc.desc   : Test HandleA2dpSuspendWhenFetch when old device unavailable and actived will change
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspendWhenFetch_008, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioStreamDeviceChangeReasonExt reason { AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE };
    AudioDeviceDescriptor actived { DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE };
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->streamStatus_ = STREAM_STATUS_STARTED;
    stream->newDeviceDescs_.emplace_back(std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE));
    streams.emplace_back(stream);
    audioCoreService->a2dpNeedSuspend_ = true;
    EXPECT_TRUE(audioCoreService->HandleA2dpSuspendWhenFetch(reason, actived, streams));
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenFetch
 * @tc.number : HandleA2dpSuspendWhenFetch_009
 * @tc.desc   : Test HandleA2dpSuspendWhenFetch when old device unavailable and actived all will change
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspendWhenFetch_009, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioStreamDeviceChangeReasonExt reason { AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE };
    AudioDeviceDescriptor actived { DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE };
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    auto streamA = std::make_shared<AudioStreamDescriptor>();
    streamA->streamStatus_ = STREAM_STATUS_STARTED;
    streamA->newDeviceDescs_.emplace_back(std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE));
    streams.emplace_back(streamA);
    auto streamB = std::make_shared<AudioStreamDescriptor>();
    streamB->streamStatus_ = STREAM_STATUS_STARTED;
    streamB->newDeviceDescs_.emplace_back(std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE));
    streams.emplace_back(streamB);
    audioCoreService->a2dpNeedSuspend_ = true;
    EXPECT_TRUE(audioCoreService->HandleA2dpSuspendWhenFetch(reason, actived, streams));
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenFetch
 * @tc.number : HandleA2dpSuspendWhenFetch_010
 * @tc.desc   : Test HandleA2dpSuspendWhenFetch when old device unavailable and the first actived will change
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspendWhenFetch_010, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioStreamDeviceChangeReasonExt reason { AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE };
    AudioDeviceDescriptor actived { DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE };
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    auto streamA = std::make_shared<AudioStreamDescriptor>();
    streamA->streamStatus_ = STREAM_STATUS_STARTED;
    streamA->newDeviceDescs_.emplace_back(std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_DP, OUTPUT_DEVICE));
    streams.emplace_back(streamA);
    auto streamB = std::make_shared<AudioStreamDescriptor>();
    streamB->streamStatus_ = STREAM_STATUS_STARTED;
    streamB->newDeviceDescs_.emplace_back(std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE));
    streams.emplace_back(streamB);
    audioCoreService->a2dpNeedSuspend_ = true;
    EXPECT_TRUE(audioCoreService->HandleA2dpSuspendWhenFetch(reason, actived, streams));
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenFetch
 * @tc.number : HandleA2dpSuspendWhenFetch_011
 * @tc.desc   : Test HandleA2dpSuspendWhenFetch when old device unavailable and the second actived will change
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspendWhenFetch_011, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioStreamDeviceChangeReasonExt reason { AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE };
    AudioDeviceDescriptor actived { DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE };
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    auto streamA = std::make_shared<AudioStreamDescriptor>();
    streamA->streamStatus_ = STREAM_STATUS_STARTED;
    streamA->newDeviceDescs_.emplace_back(std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE));
    streams.emplace_back(streamA);
    auto streamB = std::make_shared<AudioStreamDescriptor>();
    streamB->streamStatus_ = STREAM_STATUS_STARTED;
    streamB->newDeviceDescs_.emplace_back(std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_DP, OUTPUT_DEVICE));
    streams.emplace_back(streamB);
    audioCoreService->a2dpNeedSuspend_ = true;
    EXPECT_TRUE(audioCoreService->HandleA2dpSuspendWhenFetch(reason, actived, streams));
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspend
 * @tc.number : HandleA2dpSuspend_001
 * @tc.desc   : Test HandleA2dpSuspend when a2dp need suspend
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspend_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    audioCoreService->a2dpNeedSuspend_ = true;
    audioCoreService->HandleA2dpSuspend();
    EXPECT_TRUE(audioCoreService->a2dpNeedSuspend_);
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspend
 * @tc.number : HandleA2dpSuspend_003
 * @tc.desc   : Test HandleA2dpSuspend when a2dp needn't suspend after restore
 */
HWTEST_F(AudioCoreServicePrivateTest, HandleA2dpSuspend_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    audioCoreService->a2dpNeedSuspend_ = true;
    audioCoreService->HandleA2dpSuspend();
    EXPECT_TRUE(audioCoreService->a2dpNeedSuspend_);
    const uint32_t OLD_DEVICE_UNAVALIABLE_SUSPEND_MS = 1000; // 1s
    usleep(1000*OLD_DEVICE_UNAVALIABLE_SUSPEND_MS);
    audioCoreService->HandleA2dpRestore();
    EXPECT_FALSE(audioCoreService->a2dpNeedSuspend_);
}

/**
 * @tc.name   : AudioCoreServicePrivateTest_UpdateOutputRoute_001
 * @tc.number : UpdateOutputRoute_001
 * @tc.desc   : Test UpdateOutputRoute()
 */
HWTEST_F(AudioCoreServicePrivateTest, UpdateOutputRoute_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;
    streamDesc->sessionId_ = 1000;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(desc);
    std::shared_ptr<AudioRendererChangeInfo> changeInfo = std::make_shared<AudioRendererChangeInfo>();
    changeInfo->sessionId = 1000;
    changeInfo->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(changeInfo);
    audioCoreService->isRingDualToneOnPrimarySpeaker_ = true;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->AddStream(streamDesc);
    audioCoreService->pipeManager_->AddAudioPipeInfo(pipeInfo);

    uint32_t pipeId = DEFAULT_PIPE_ID;
    audioCoreService->UpdateOutputRoute(streamDesc, pipeId);
    EXPECT_EQ(1, audioCoreService->streamsWhenRingDualOnPrimarySpeaker_.size());

    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
    audioCoreService->streamsWhenRingDualOnPrimarySpeaker_.clear();
    audioCoreService->pipeManager_->RemoveAudioPipeInfo(pipeInfo);
}

/**
 * @tc.name   : Test AudioCoreService::DeactivateA2dpAfterExclude
 * @tc.number : DeactivateA2dpAfterExclude_001
 * @tc.desc   : Test DeactivateA2dpAfterExclude when no A2DP device in exclude list
 */
HWTEST_F(AudioCoreServicePrivateTest, DeactivateA2dpAfterExclude_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    // Create exclude device list without A2DP device
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> excludeDeviceDescs;
    auto speakerDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    excludeDeviceDescs.push_back(speakerDevice);

    // Create output stream list
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    outputStreamDescs.push_back(streamDesc);

    // Method should return false as there's no A2DP device in exclude list
    bool result = audioCoreService->DeactivateA2dpAfterExclude(excludeDeviceDescs, outputStreamDescs);
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test AudioCoreService::DeactivateA2dpAfterExclude
 * @tc.number : DeactivateA2dpAfterExclude_002
 * @tc.desc   : Test DeactivateA2dpAfterExclude when A2DP device in exclude list but stream will use A2DP
 */
HWTEST_F(AudioCoreServicePrivateTest, DeactivateA2dpAfterExclude_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    // Create exclude device list with A2DP device
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> excludeDeviceDescs;
    auto a2dpDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    excludeDeviceDescs.push_back(a2dpDevice);

    // Create output stream that will use A2DP
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    auto newA2dpDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(newA2dpDevice);
    outputStreamDescs.push_back(streamDesc);

    // Method should return false as stream will use A2DP
    bool result = audioCoreService->DeactivateA2dpAfterExclude(excludeDeviceDescs, outputStreamDescs);
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test AudioCoreService::DeactivateA2dpAfterExclude
 * @tc.number : DeactivateA2dpAfterExclude_003
 * @tc.desc   : Test DeactivateA2dpAfterExclude when A2DP device in exclude list and no stream will use A2DP
 */
HWTEST_F(AudioCoreServicePrivateTest, DeactivateA2dpAfterExclude_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    // Create exclude device list with A2DP device
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> excludeDeviceDescs;
    auto a2dpDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    excludeDeviceDescs.push_back(a2dpDevice);

    // Create output stream that will not use A2DP
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    auto speakerDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(speakerDevice);
    outputStreamDescs.push_back(streamDesc);

    // Method should return true and deactivate A2DP
    bool result = audioCoreService->DeactivateA2dpAfterExclude(excludeDeviceDescs, outputStreamDescs);
    EXPECT_TRUE(result);
}

/**
 * @tc.name   : Test AudioCoreService::DeactivateA2dpAfterExclude
 * @tc.number : DeactivateA2dpAfterExclude_004
 * @tc.desc   : Test DeactivateA2dpAfterExclude with empty exclude list
 */
HWTEST_F(AudioCoreServicePrivateTest, DeactivateA2dpAfterExclude_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    // Create empty exclude device list
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> excludeDeviceDescs;

    // Create output stream list
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    outputStreamDescs.push_back(streamDesc);

    // Method should return false as there's no A2DP device in exclude list
    bool result = audioCoreService->DeactivateA2dpAfterExclude(excludeDeviceDescs, outputStreamDescs);
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test AudioCoreService::DeactivateA2dpAfterExclude
 * @tc.number : DeactivateA2dpAfterExclude_005
 * @tc.desc   : Test DeactivateA2dpAfterExclude with empty output stream list
 */
HWTEST_F(AudioCoreServicePrivateTest, DeactivateA2dpAfterExclude_005, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    // Create exclude device list with A2DP device
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> excludeDeviceDescs;
    auto a2dpDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    excludeDeviceDescs.push_back(a2dpDevice);

    // Create empty output stream list
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs;

    // Method should return true as no stream will use A2DP
    bool result = audioCoreService->DeactivateA2dpAfterExclude(excludeDeviceDescs, outputStreamDescs);
    EXPECT_TRUE(result);
}

/**
 * @tc.name   : Test AudioCoreService.
 * @tc.number : GetFlagForUltraFastStream_001
 * @tc.desc   : Test AudioCoreService::GetFlagForUltraFastStream with empty newDeviceDescs
 */
HWTEST_F(AudioCoreServicePrivateTest, GetFlagForUltraFastStream_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    bool isCreateProcess = true;

    audioCoreService->GetFlagForUltraFastStream(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_FLAG_NONE);
}

/**
 * @tc.name   : Test AudioCoreService.
 * @tc.number : GetFlagForUltraFastStream_003
 * @tc.desc   : Test AudioCoreService::GetFlagForUltraFastStream when ultra fast is not supported
 */
HWTEST_F(AudioCoreServicePrivateTest, GetFlagForUltraFastStream_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->isSupportUltraFast_ = false;

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    bool isCreateProcess = true;

    audioCoreService->GetFlagForUltraFastStream(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_FLAG_NONE);
}

/**
 * @tc.name   : Test AudioCoreService.
 * @tc.number : GetFlagForUltraFastStream_004
 * @tc.desc   : Test AudioCoreService::GetFlagForUltraFastStream when bundle name is not in support list
 */
HWTEST_F(AudioCoreServicePrivateTest, GetFlagForUltraFastStream_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->isSupportUltraFast_ = true;

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    streamDesc->bundleName_ = "test.bundle.name";
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    bool isCreateProcess = true;

    audioCoreService->GetFlagForUltraFastStream(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_FLAG_NONE);
}

/**
 * @tc.name   : Test AudioCoreService.
 * @tc.number : DecidedAudioFlagForStreams_001
 * @tc.desc   : Test AudioCoreService::DecidedAudioFlagForStreams with empty streamDescs
 */
HWTEST_F(AudioCoreServicePrivateTest, DecidedAudioFlagForStreams_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;

    audioCoreService->DecidedAudioFlagForStreams(streamDescs);
    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name   : Test AudioCoreService.
 * @tc.number : DecidedAudioFlagForStreams_002
 * @tc.desc   : Test AudioCoreService::DecidedAudioFlagForStreams with empty streamDescs
 */
HWTEST_F(AudioCoreServicePrivateTest, DecidedAudioFlagForStreams_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioFlag fastFlag = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc->sessionId_ = 1001;
    streamDesc->SetAudioFlag(fastFlag);
    streamDescs.push_back(streamDesc);

    audioCoreService->DecidedAudioFlagForStreams(streamDescs);
    EXPECT_EQ(streamDesc->GetAudioFlag(), AUDIO_FLAG_MMAP);
}

/**
 * @tc.name   : Test AudioCoreService.
 * @tc.number : DecidedAudioFlagForStreams_003
 * @tc.desc   : Test AudioCoreService::DecidedAudioFlagForStreams with empty streamDescs
 */
HWTEST_F(AudioCoreServicePrivateTest, DecidedAudioFlagForStreams_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc1->newDeviceDescs_.push_back(deviceDesc);
    AudioFlag fastFlag = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc1->sessionId_ = 1001;
    streamDesc1->SetAudioFlag(fastFlag);
    streamDesc1->SetFastStreamForcedNormalFlag(true);
    streamDescs.push_back(streamDesc1);

    std::shared_ptr<AudioStreamDescriptor> streamDesc2 = std::make_shared<AudioStreamDescriptor>();
    streamDesc2->newDeviceDescs_.push_back(deviceDesc);
    streamDesc1->sessionId_ = 1002;
    streamDesc1->SetAudioFlag(fastFlag);
    streamDescs.push_back(streamDesc2);

    audioCoreService->DecidedAudioFlagForStreams(streamDescs);
    EXPECT_EQ(streamDesc1->GetAudioFlag(), AUDIO_OUTPUT_FLAG_NORMAL);
    EXPECT_EQ(streamDesc2->GetAudioFlag(), AUDIO_OUTPUT_FLAG_NORMAL);
    EXPECT_FALSE(streamDesc1->GetFastStreamForcedNormalFlag());
}

/**
 * @tc.name  : AudioCoreServicePrivateTest_WriteScoStateFaultEvent_001
 * @tc.number: WriteScoStateFaultEvent_001
 * @tc.desc  : Test AudioCoreService::WriteScoStateFaultEvent()
 */
HWTEST_F(AudioCoreServicePrivateTest, WriteScoStateFaultEvent_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    deviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioStreamDescriptor->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    audioCoreService->WriteScoStateFaultEvent(deviceDescriptor);
    int ret = HiSysEventWrite(
        HiviewDFX::HiSysEvent::Domain::AUDIO, "STREAM_STATE_SCO",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "DEVICE_TYPE", static_cast<int32_t>(deviceDescriptor->deviceType_),
        "STREAM_USAGE", static_cast<int32_t>(audioStreamDescriptor->rendererInfo_.streamUsage)
    );
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : SetCapturerMuteHint_001
 * @tc.number: SetCapturerMuteHint_001
 * @tc.desc  : Test AudioCoreService::SetCapturerMuteHint returns invalid param when session does not exist.
 */
HWTEST_F(AudioCoreServicePrivateTest, SetCapturerMuteHint_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    audioCoreService->streamCollector_.audioCapturerChangeInfos_.clear();
    audioCoreService->audioEffectService_.appConfiguredCapturerMuteHint_.clear();
    audioCoreService->audioEffectService_.appConfiguredSessionMuteHint_.clear();

    int32_t ret = audioCoreService->SetCapturerMuteHint(7001, true);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : SetCapturerMuteHint_002
 * @tc.number: SetCapturerMuteHint_002
 * @tc.desc  : Test AudioCoreService::SetCapturerMuteHint returns illegal state when capturer is not running.
 */
HWTEST_F(AudioCoreServicePrivateTest, SetCapturerMuteHint_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    audioCoreService->streamCollector_.audioCapturerChangeInfos_.clear();
    audioCoreService->audioEffectService_.appConfiguredCapturerMuteHint_.clear();
    audioCoreService->audioEffectService_.appConfiguredSessionMuteHint_.clear();

    auto capturerInfo = std::make_shared<AudioCapturerChangeInfo>();
    ASSERT_NE(capturerInfo, nullptr);
    capturerInfo->sessionId = 7002;
    capturerInfo->capturerState = CAPTURER_STOPPED;
    capturerInfo->capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    audioCoreService->streamCollector_.audioCapturerChangeInfos_.push_back(capturerInfo);

    int32_t ret = audioCoreService->SetCapturerMuteHint(7002, true);
    EXPECT_EQ(ret, ERROR_ILLEGAL_STATE);
}

/**
 * @tc.name  : SetCapturerMuteHint_003
 * @tc.number: SetCapturerMuteHint_003
 * @tc.desc  : Test AudioCoreService::SetCapturerMuteHint succeeds for running session and updates effect state.
 */
HWTEST_F(AudioCoreServicePrivateTest, SetCapturerMuteHint_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    audioCoreService->streamCollector_.audioCapturerChangeInfos_.clear();
    audioCoreService->audioEffectService_.appConfiguredCapturerMuteHint_.clear();
    audioCoreService->audioEffectService_.appConfiguredSessionMuteHint_.clear();

    auto capturerInfo = std::make_shared<AudioCapturerChangeInfo>();
    ASSERT_NE(capturerInfo, nullptr);
    capturerInfo->sessionId = 7003;
    capturerInfo->capturerState = CAPTURER_RUNNING;
    capturerInfo->capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    audioCoreService->streamCollector_.audioCapturerChangeInfos_.push_back(capturerInfo);

    int32_t ret = audioCoreService->SetCapturerMuteHint(7003, true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(audioCoreService->audioEffectService_.GetEffectiveAppMuteHint(7003));
}

/**
 * @tc.name  : ReEvaluateVoipBypassByAppMuteHint_001
 * @tc.number: ReEvaluateVoipBypassByAppMuteHint_001
 * @tc.desc  : Test AudioCoreService::ReEvaluateVoipBypassByAppMuteHint prunes stale mute hint state.
 */
HWTEST_F(AudioCoreServicePrivateTest, ReEvaluateVoipBypassByAppMuteHint_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    audioCoreService->streamCollector_.audioCapturerChangeInfos_.clear();
    audioCoreService->audioEffectService_.appConfiguredCapturerMuteHint_.clear();
    audioCoreService->audioEffectService_.appConfiguredSessionMuteHint_.clear();

    audioCoreService->audioEffectService_.SetAppConfiguredCapturerMuteHint(7010, true);
    EXPECT_TRUE(audioCoreService->audioEffectService_.GetEffectiveAppMuteHint(7010));

    audioCoreService->ReEvaluateVoipBypassByAppMuteHint();
    EXPECT_FALSE(audioCoreService->audioEffectService_.GetEffectiveAppMuteHint(7010));
}

/**
 * @tc.name  : AudioCoreService_CalculatePipeAudioScene_001
 * @tc.desc  : Test CalculatePipeAudioScene with empty pipe
 * @tc.type  : FUNC
 */
HWTEST_F(AudioCoreServicePrivateTest, CalculatePipeAudioScene_001, TestSize.Level1)
{
    AudioScene scene = testCoreService_->CalculatePipeAudioScene(DEFAULT_PIPE_ID);
    EXPECT_EQ(scene, AUDIO_SCENE_DEFAULT);
}

/**
 * @tc.name  : AudioCoreService_TrackScoStreamAudioScene_001
 * @tc.desc  : Test TrackScoStreamAudioScene with cellular call
 * @tc.type  : FUNC
 */
HWTEST_F(AudioCoreServicePrivateTest, TrackScoStreamAudioScene_001, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    streamDesc->sessionId_ = TEST_STREAM_1_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->callerUid_ = 1001;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;

    testCoreService_->TrackScoStreamAudioScene(streamDesc, true);
    EXPECT_TRUE(ScoAudioSceneManager::GetInstance().HasScoStream());

    testCoreService_->TrackScoStreamAudioScene(streamDesc, false);
    EXPECT_FALSE(ScoAudioSceneManager::GetInstance().HasScoStream());
}

/**
 * @tc.name  : AudioCoreService_TrackScoStreamAudioScene_002
 * @tc.desc  : Test TrackScoStreamAudioScene with voip
 * @tc.type  : FUNC
 */
HWTEST_F(AudioCoreServicePrivateTest, TrackScoStreamAudioScene_002, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    streamDesc->sessionId_ = TEST_STREAM_2_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->callerUid_ = 1002;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;

    testCoreService_->TrackScoStreamAudioScene(streamDesc, true);
    EXPECT_TRUE(ScoAudioSceneManager::GetInstance().HasScoStream());

    AudioScene highestScene = ScoAudioSceneManager::GetInstance().GetHighestPriorityAudioScene();
    EXPECT_EQ(highestScene, AUDIO_SCENE_PHONE_CHAT);

    ScoAudioSceneManager::GetInstance().ClearAllScoStreams();
}

/**
 * @tc.name  : AudioCoreService_TrackScoStreamAudioScene_003
 * @tc.desc  : Test TrackScoStreamAudioScene with recognition
 * @tc.type  : FUNC
 */
HWTEST_F(AudioCoreServicePrivateTest, TrackScoStreamAudioScene_003, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    streamDesc->sessionId_ = TEST_STREAM_1_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->callerUid_ = 1003;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;

    testCoreService_->TrackScoStreamAudioScene(streamDesc, true);
    EXPECT_TRUE(ScoAudioSceneManager::GetInstance().HasScoStream());

    ScoAudioSceneManager::GetInstance().ClearAllScoStreams();
}

/**
 * @tc.name  : AudioCoreService_TrackScoStreamAudioScene_004
 * @tc.desc  : Test TrackScoStreamAudioScene priority ordering
 * @tc.type  : FUNC
 */
HWTEST_F(AudioCoreServicePrivateTest, TrackScoStreamAudioScene_004, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> voipStream = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(voipStream, nullptr);
    voipStream->sessionId_ = TEST_STREAM_1_SESSION_ID;
    voipStream->audioMode_ = AUDIO_MODE_PLAYBACK;
    voipStream->streamStatus_ = STREAM_STATUS_STARTED;
    voipStream->callerUid_ = 1001;
    voipStream->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;

    std::shared_ptr<AudioStreamDescriptor> cellularStream = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(cellularStream, nullptr);
    cellularStream->sessionId_ = TEST_STREAM_2_SESSION_ID;
    cellularStream->audioMode_ = AUDIO_MODE_PLAYBACK;
    cellularStream->streamStatus_ = STREAM_STATUS_STARTED;
    cellularStream->callerUid_ = 1002;
    cellularStream->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;

    testCoreService_->TrackScoStreamAudioScene(voipStream, true);
    testCoreService_->TrackScoStreamAudioScene(cellularStream, true);

    AudioScene highestScene = ScoAudioSceneManager::GetInstance().GetHighestPriorityAudioScene();
    EXPECT_EQ(highestScene, AUDIO_SCENE_PHONE_CALL);

    ScoAudioSceneManager::GetInstance().ClearAllScoStreams();
}

} // namespace AudioStandard
} // namespace OHOS
