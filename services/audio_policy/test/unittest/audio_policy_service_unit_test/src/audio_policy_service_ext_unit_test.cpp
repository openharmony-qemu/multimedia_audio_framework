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
#include "audio_policy_service_ext_unit_test.h"
#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_adapter_info.h"
#include "audio_module_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_policy_manager.h"
#include "audio_session_info.h"
#include "audio_system_manager.h"
#include <memory>
#include <thread>
#include <vector>
#ifdef BLUE_YELLOW_DIFF
#include "audio_ec_info.h"
#endif

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace AudioStandard {
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
bool g_isInit = false;
static const std::string PIPE_PRIMARY_OUTPUT_UNITTEST = "primary_output";
static const std::string PIPE_PRIMARY_INPUT_UNITTEST = "primary_input";
static const std::string PIPE_USB_ARM_OUTPUT_UNITTEST = "usb_arm_output";
static const std::string PIPE_DP_OUTPUT_UNITTEST = "dp_output";
static const std::string PIPE_USB_ARM_INPUT_UNITTEST = "usb_arm_input";

static AudioPolicyServer g_server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
void AudioPolicyServiceExtUnitTest::SetUpTestCase(void) {}
void AudioPolicyServiceExtUnitTest::TearDownTestCase(void) {}
void AudioPolicyServiceExtUnitTest::SetUp(void) {}
void AudioPolicyServiceExtUnitTest::TearDown(void)
{
    g_server.OnStop();
    g_isInit = false;
}

AudioPolicyServer *GetServerPtr()
{
    if (!g_isInit) {
        g_server.OnStart();
        g_server.OnAddSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "");
        g_server.OnAddSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, "");
        g_server.OnAddSystemAbility(ACCESSIBILITY_MANAGER_SERVICE_ID, "");
        g_server.OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
        g_server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
        g_server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
        g_server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        g_server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        g_server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        g_server.audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_isInit = true;
    }
    return &g_server;
}

/**
 * @tc.name  : Test AudioVolumeDump.
 * @tc.number: AudioVolumeDump_001
 * @tc.desc  : Test AudioVolumeDump interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, AudioVolumeDump_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string dumpString = "666";
    server->AudioVolumeDump(dumpString);
    EXPECT_NE(dumpString, "666");
}

/**
 * @tc.name  : Test AudioStreamDump.
 * @tc.number: AudioStreamDump_001
 * @tc.desc  : Test AudioStreamDump interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, AudioStreamDump_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string dumpString = "666";
    server->AudioStreamDump(dumpString);
    EXPECT_NE(dumpString, "666");
}

/**
 * @tc.name  : Test CheckAudioSessionStrategy.
 * @tc.number: CheckAudioSessionStrategy_001
 * @tc.desc  : Test CheckAudioSessionStrategy interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, CheckAudioSessionStrategy_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    AudioSessionStrategy sessionStrategy;
    sessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    bool ret = server->CheckAudioSessionStrategy(sessionStrategy);
    EXPECT_EQ(ret, true);

    sessionStrategy.concurrencyMode = (AudioConcurrencyMode)666;
    ret = server->CheckAudioSessionStrategy(sessionStrategy);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test LoadSplitModule.
 * @tc.number: LoadSplitModule_001
 * @tc.desc  : Test LoadSplitModule interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, LoadSplitModule_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string splitArgs = "";
    std::string networkId = "";
    int32_t ret = server->audioPolicyService_.LoadSplitModule(splitArgs, networkId);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    splitArgs = "test";
    networkId = LOCAL_NETWORK_ID;
    ret = server->audioPolicyService_.LoadSplitModule(splitArgs, networkId);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test HandleA2dpDeviceInOffload.
 * @tc.number: HandleA2dpDeviceInOffload_001
 * @tc.desc  : Test HandleA2dpDeviceInOffload interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, HandleA2dpDeviceInOffload_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    BluetoothOffloadState a2dpOffloadFlag = A2DP_NOT_OFFLOAD;
    int32_t ret = server->audioPolicyService_.HandleA2dpDeviceInOffload(a2dpOffloadFlag);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ReconfigureAudioChannel.
 * @tc.number: ReconfigureAudioChannel_001
 * @tc.desc  : Test ReconfigureAudioChannel interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, ReconfigureAudioChannel_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    uint32_t channelCount = 1;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    int32_t ret = server->audioPolicyService_.ReconfigureAudioChannel(channelCount, deviceType);
    EXPECT_EQ(ret, ERROR);

    deviceType = DeviceType::DEVICE_TYPE_FILE_SINK;
    ret = server->audioPolicyService_.ReconfigureAudioChannel(channelCount, deviceType);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test WriteDeviceChangedSysEvents.
 * @tc.number: WriteDeviceChangedSysEvents_001
 * @tc.desc  : Test WriteDeviceChangedSysEvents interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, WriteDeviceChangedSysEvents_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::vector<sptr<AudioDeviceDescriptor>> desc =
        AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::ALL_DEVICES_FLAG);
    bool isConnected = false;
    server->audioPolicyService_.WriteDeviceChangedSysEvents(desc, isConnected);
    EXPECT_EQ(isConnected, false);
}

/**
 * @tc.name  : Test UpdateTrackerDeviceChange.
 * @tc.number: UpdateTrackerDeviceChange_001
 * @tc.desc  : Test UpdateTrackerDeviceChange interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, UpdateTrackerDeviceChange_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::vector<sptr<AudioDeviceDescriptor>> desc =
        AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::ALL_DEVICES_FLAG);
    server->audioPolicyService_.UpdateTrackerDeviceChange(desc);
    EXPECT_TRUE(desc.size() >= 0);
}

/**
 * @tc.name  : Test SetAbsVolumeSceneAsync.
 * @tc.number: SetAbsVolumeSceneAsync_001
 * @tc.desc  : Test SetAbsVolumeSceneAsync interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, SetAbsVolumeSceneAsync_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string macAddress = "";
    bool support = false;
    server->audioPolicyService_.SetAbsVolumeSceneAsync(macAddress, support);
    EXPECT_EQ(support, false);
}

/**
 * @tc.name  : Test SetDeviceAbsVolumeSupported.
 * @tc.number: SetDeviceAbsVolumeSupported_001
 * @tc.desc  : Test SetDeviceAbsVolumeSupported interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, SetDeviceAbsVolumeSupported_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string macAddress = "";
    bool support = false;
    int32_t ret = server->audioPolicyService_.SetDeviceAbsVolumeSupported(macAddress, support);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test IsWiredHeadSet.
 * @tc.number: IsWiredHeadSet_001
 * @tc.desc  : Test IsWiredHeadSet interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, IsWiredHeadSet_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    DeviceType deviceType = DeviceType::DEVICE_TYPE_MAX;
    bool ret = server->audioPolicyService_.IsWiredHeadSet(deviceType);
    EXPECT_EQ(ret, false);

    deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    ret = server->audioPolicyService_.IsWiredHeadSet(deviceType);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsBlueTooth.
 * @tc.number: IsBlueTooth_001
 * @tc.desc  : Test IsBlueTooth interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, IsBlueTooth_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    bool ret = server->audioPolicyService_.IsBlueTooth(deviceType);
    EXPECT_EQ(ret, false);

    deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = server->audioPolicyService_.IsBlueTooth(deviceType);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test CheckBlueToothActiveMusicTime.
 * @tc.number: CheckBlueToothActiveMusicTime_001
 * @tc.desc  : Test CheckBlueToothActiveMusicTime interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, CheckBlueToothActiveMusicTime_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    int32_t safeVolume = 1;
    server->audioPolicyService_.CheckBlueToothActiveMusicTime(safeVolume);
    EXPECT_EQ(safeVolume, 1);
}

/**
 * @tc.name  : Test CheckWiredActiveMusicTime.
 * @tc.number: CheckWiredActiveMusicTime_001
 * @tc.desc  : Test CheckWiredActiveMusicTime interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, CheckWiredActiveMusicTime_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    int32_t safeVolume = 1;
    server->audioPolicyService_.CheckWiredActiveMusicTime(safeVolume);
    EXPECT_EQ(safeVolume, 1);
}

/**
 * @tc.name  : Test RestoreSafeVolume.
 * @tc.number: RestoreSafeVolume_001
 * @tc.desc  : Test RestoreSafeVolume interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, RestoreSafeVolume_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    AudioStreamType streamType = AudioStreamType::STREAM_RING;
    int32_t safeVolume = 1;
    server->audioPolicyService_.RestoreSafeVolume(streamType, safeVolume);
    EXPECT_EQ(safeVolume, 1);
}

/**
 * @tc.name  : Test CreateCheckMusicActiveThread.
 * @tc.number: CreateCheckMusicActiveThread_001
 * @tc.desc  : Test CreateCheckMusicActiveThread interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, CreateCheckMusicActiveThread_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    server->audioPolicyService_.CreateCheckMusicActiveThread();
}

/**
 * @tc.name  : Test DealWithSafeVolume.
 * @tc.number: DealWithSafeVolume_001
 * @tc.desc  : Test DealWithSafeVolume interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, DealWithSafeVolume_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    int32_t volumeLevel = 5;
    bool isA2dpDevice = true;
    int32_t volumeLevelRet = server->audioPolicyService_.DealWithSafeVolume(volumeLevel, isA2dpDevice);
    EXPECT_EQ(volumeLevelRet, 5);

    isA2dpDevice = false;
    volumeLevelRet = server->audioPolicyService_.DealWithSafeVolume(volumeLevel, isA2dpDevice);
    EXPECT_EQ(volumeLevelRet, 8);

    volumeLevel = 11;
    volumeLevelRet = server->audioPolicyService_.DealWithSafeVolume(volumeLevel, isA2dpDevice);
    EXPECT_EQ(volumeLevelRet, 8);
}

/**
 * @tc.name  : Test HandleAbsBluetoothVolume.
 * @tc.number: HandleAbsBluetoothVolume_001
 * @tc.desc  : Test HandleAbsBluetoothVolume interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, HandleAbsBluetoothVolume_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string macAddress = "";
    int32_t volumeLevel = 10;
    int32_t safeVolumeLevel = server->audioPolicyService_.HandleAbsBluetoothVolume(macAddress, volumeLevel);
    EXPECT_EQ(safeVolumeLevel, 8);

    volumeLevel = 1;
    safeVolumeLevel = server->audioPolicyService_.HandleAbsBluetoothVolume(macAddress, volumeLevel);
    EXPECT_EQ(safeVolumeLevel, 1);
}

/**
 * @tc.name  : Test SetA2dpDeviceVolume.
 * @tc.number: SetA2dpDeviceVolume_001
 * @tc.desc  : Test SetA2dpDeviceVolume interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, SetA2dpDeviceVolume_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string macAddress = "";
    int32_t volumeLevel = 4;
    bool internalCall = false;
    int32_t ret = server->audioPolicyService_.SetA2dpDeviceVolume(macAddress, volumeLevel, internalCall);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test TriggerDeviceChangedCallback.
 * @tc.number: TriggerDeviceChangedCallback_001
 * @tc.desc  : Test TriggerDeviceChangedCallback interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, TriggerDeviceChangedCallback_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    vector<sptr<AudioDeviceDescriptor>> desc =
        AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::ALL_DEVICES_FLAG);
    bool isConnected = false;
    server->audioPolicyService_.TriggerDeviceChangedCallback(desc, isConnected);
    EXPECT_EQ(isConnected, false);
}

/**
 * @tc.name  : Test GetDeviceRole.
 * @tc.number: GetDeviceRole_001
 * @tc.desc  : Test GetDeviceRole interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetDeviceRole_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    DeviceRole deviceRole = server->audioPolicyService_.GetDeviceRole(deviceType);
    EXPECT_EQ(deviceRole, DeviceRole::OUTPUT_DEVICE);

    deviceType = DeviceType::DEVICE_TYPE_WAKEUP;
    deviceRole = server->audioPolicyService_.GetDeviceRole(deviceType);
    EXPECT_EQ(deviceRole, DeviceRole::INPUT_DEVICE);

    deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
    deviceRole = server->audioPolicyService_.GetDeviceRole(deviceType);
    EXPECT_EQ(deviceRole, DeviceRole::DEVICE_ROLE_NONE);
}

/**
 * @tc.name  : Test GetDeviceRole.
 * @tc.number: GetDeviceRole_002
 * @tc.desc  : Test GetDeviceRole interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetDeviceRole_002, TestSize.Level1)
{
    auto server = GetServerPtr();
    AudioPin pin = AudioPin::AUDIO_PIN_NONE;
    DeviceRole deviceRole = server->audioPolicyService_.GetDeviceRole(pin);
    EXPECT_EQ(deviceRole, DeviceRole::DEVICE_ROLE_NONE);

    pin = AudioPin::AUDIO_PIN_OUT_SPEAKER;
    deviceRole = server->audioPolicyService_.GetDeviceRole(pin);
    EXPECT_EQ(deviceRole, DeviceRole::OUTPUT_DEVICE);

    pin = AudioPin::AUDIO_PIN_IN_MIC;
    deviceRole = server->audioPolicyService_.GetDeviceRole(pin);
    EXPECT_EQ(deviceRole, DeviceRole::INPUT_DEVICE);

    pin = (AudioPin)666;
    deviceRole = server->audioPolicyService_.GetDeviceRole(pin);
    EXPECT_EQ(deviceRole, DeviceRole::DEVICE_ROLE_NONE);
}

/**
 * @tc.name  : Test GetVoipRendererFlag.
 * @tc.number: GetVoipRendererFlag_001
 * @tc.desc  : Test GetVoipRendererFlag interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetVoipRendererFlag_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string sinkPortName = "";
    std::string networkId = "";
    int32_t ret;

    server->audioPolicyService_.normalVoipFlag_ = false;
    server->audioPolicyService_.enableFastVoip_ = false;
    ret = server->audioPolicyService_.GetVoipRendererFlag(sinkPortName, networkId);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    networkId = LOCAL_NETWORK_ID;
    server->audioPolicyService_.normalVoipFlag_ = true;
    server->audioPolicyService_.enableFastVoip_ = true;
    ret = server->audioPolicyService_.GetVoipRendererFlag(sinkPortName, networkId);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    sinkPortName = PRIMARY_SPEAKER;
    networkId = REMOTE_NETWORK_ID;
    server->audioPolicyService_.enableFastVoip_ = false;
    server->audioPolicyService_.normalVoipFlag_ = false;
    ret = server->audioPolicyService_.GetVoipRendererFlag(sinkPortName, networkId);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    sinkPortName = BLUETOOTH_SPEAKER;
    networkId = REMOTE_NETWORK_ID;
    server->audioPolicyService_.enableFastVoip_ = false;
    server->audioPolicyService_.normalVoipFlag_ = false;
    ret = server->audioPolicyService_.GetVoipRendererFlag(sinkPortName, networkId);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    sinkPortName = PRIMARY_SPEAKER;
    networkId = LOCAL_NETWORK_ID;
    server->audioPolicyService_.enableFastVoip_ = true;
    ret = server->audioPolicyService_.GetVoipRendererFlag(sinkPortName, networkId);
    EXPECT_EQ(ret, AUDIO_FLAG_VOIP_FAST);

    sinkPortName = USB_SPEAKER;
    networkId = REMOTE_NETWORK_ID;
    ret = server->audioPolicyService_.GetVoipRendererFlag(sinkPortName, networkId);
    EXPECT_EQ(ret, AUDIO_FLAG_VOIP_FAST);

    sinkPortName = PRIMARY_SPEAKER;
    networkId = LOCAL_NETWORK_ID;
    server->audioPolicyService_.enableFastVoip_ = false;
    server->audioPolicyService_.normalVoipFlag_ = false;
    ret = server->audioPolicyService_.GetVoipRendererFlag(sinkPortName, networkId);
    EXPECT_EQ(ret, AUDIO_FLAG_VOIP_DIRECT);
}

/**
 * @tc.name  : Test UpdateInputDeviceInfo.
 * @tc.number: UpdateInputDeviceInfo_001
 * @tc.desc  : Test UpdateInputDeviceInfo interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, UpdateInputDeviceInfo_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    server->audioPolicyService_.UpdateInputDeviceInfo(deviceType);
    EXPECT_EQ(server->audioPolicyService_.currentActiveInputDevice_.deviceType_, DeviceType::DEVICE_TYPE_MIC);

    deviceType = DeviceType::DEVICE_TYPE_FILE_SINK;
    server->audioPolicyService_.UpdateInputDeviceInfo(deviceType);
    EXPECT_EQ(server->audioPolicyService_.currentActiveInputDevice_.deviceType_, DeviceType::DEVICE_TYPE_FILE_SOURCE);

    deviceType = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;
    server->audioPolicyService_.UpdateInputDeviceInfo(deviceType);
    EXPECT_EQ(server->audioPolicyService_.currentActiveInputDevice_.deviceType_, DeviceType::DEVICE_TYPE_USB_HEADSET);

    deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    server->audioPolicyService_.UpdateInputDeviceInfo(deviceType);
    EXPECT_EQ(server->audioPolicyService_.currentActiveInputDevice_.deviceType_, DeviceType::DEVICE_TYPE_WIRED_HEADSET);

    deviceType = (DeviceType)777;
    server->audioPolicyService_.UpdateInputDeviceInfo(deviceType);
    EXPECT_EQ(server->audioPolicyService_.currentActiveInputDevice_.deviceType_, DeviceType::DEVICE_TYPE_WIRED_HEADSET);
}


/**
 * @tc.name  : Test GetDeviceTypeFromPin.
 * @tc.number: GetDeviceTypeFromPin_001
 * @tc.desc  : Test GetDeviceTypeFromPin interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetDeviceTypeFromPin_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    AudioPin hdiPin;
    hdiPin = AudioPin::AUDIO_PIN_NONE;
    DeviceType deviceType = server->audioPolicyService_.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DeviceType::DEVICE_TYPE_DEFAULT);

    hdiPin = AudioPin::AUDIO_PIN_OUT_SPEAKER;
    deviceType = server->audioPolicyService_.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DeviceType::DEVICE_TYPE_SPEAKER);

    hdiPin = AudioPin::AUDIO_PIN_OUT_USB_HEADSET;
    deviceType = server->audioPolicyService_.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DeviceType::DEVICE_TYPE_USB_ARM_HEADSET);

    hdiPin = AudioPin::AUDIO_PIN_IN_MIC;
    deviceType = server->audioPolicyService_.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DeviceType::DEVICE_TYPE_MIC);

    hdiPin = (AudioPin)666;
    deviceType = server->audioPolicyService_.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DeviceType::DEVICE_TYPE_DEFAULT);
}

/**
 * @tc.name  : Test GetProcessDeviceInfo.
 * @tc.number: GetProcessDeviceInfo_001
 * @tc.desc  : Test GetProcessDeviceInfo interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetProcessDeviceInfo_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    AudioProcessConfig config;
    DeviceInfo deviceInfo;
    int32_t ret;
    
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    ret = server->audioPolicyService_.GetProcessDeviceInfo(config, deviceInfo);
    EXPECT_EQ(ret, ERROR);

    config.rendererInfo.streamUsage = STREAM_USAGE_VIDEO_COMMUNICATION;
    ret = server->audioPolicyService_.GetProcessDeviceInfo(config, deviceInfo);
    EXPECT_EQ(ret, ERROR);

    config.rendererInfo.streamUsage = STREAM_USAGE_UNKNOWN;
    ret = server->audioPolicyService_.GetProcessDeviceInfo(config, deviceInfo);
    EXPECT_EQ(ret, SUCCESS);

    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    ret = server->audioPolicyService_.GetProcessDeviceInfo(config, deviceInfo);
    EXPECT_EQ(ret, ERROR);

    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    ret = server->audioPolicyService_.GetProcessDeviceInfo(config, deviceInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetVoipDeviceInfo.
 * @tc.number: GetVoipDeviceInfo_001
 * @tc.desc  : Test GetVoipDeviceInfo interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetVoipDeviceInfo_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    AudioRendererInfo rendererInfo;
    AudioProcessConfig config;
    DeviceInfo deviceInfo;
    deviceInfo.deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    std::vector<sptr<AudioDeviceDescriptor>> preferredDeviceList =
        AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo);
    int32_t type;
    int32_t ret;
    
    type = AUDIO_FLAG_NORMAL;
    ret = server->audioPolicyService_.GetVoipDeviceInfo(config, deviceInfo, type, preferredDeviceList);
    EXPECT_EQ(ret, ERROR);

    type = AUDIO_FLAG_DIRECT;
    config.streamInfo.samplingRate = SAMPLE_RATE_8000;
    ret = server->audioPolicyService_.GetVoipDeviceInfo(config, deviceInfo, type, preferredDeviceList);
    EXPECT_EQ(ret, SUCCESS);

    config.streamInfo.samplingRate = SAMPLE_RATE_192000;
    ret = server->audioPolicyService_.GetVoipDeviceInfo(config, deviceInfo, type, preferredDeviceList);
    EXPECT_EQ(ret, SUCCESS);

    type = AUDIO_FLAG_VOIP_DIRECT;
    ret = server->audioPolicyService_.GetVoipDeviceInfo(config, deviceInfo, type, preferredDeviceList);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetSharedVolume.
 * @tc.number: GetSharedVolume_001
 * @tc.desc  : Test GetSharedVolume interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetSharedVolume_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    AudioVolumeType streamType = AudioStreamType::STREAM_RING;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    Volume vol;
    
    bool ret = server->audioPolicyService_.GetSharedVolume(streamType, deviceType, vol);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test UpdateAudioCapturerMicrophoneDescriptor.
 * @tc.number: UpdateAudioCapturerMicrophoneDescriptor_001
 * @tc.desc  : Test UpdateAudioCapturerMicrophoneDescriptor interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, UpdateAudioCapturerMicrophoneDescriptor_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    DeviceType devType = DeviceType::DEVICE_TYPE_NONE;
    int32_t sessionId = 0;
    sptr<AudioDeviceDescriptor> deviceDescriptor = new (std::nothrow) AudioDeviceDescriptor();

    server->audioPolicyService_.AddMicrophoneDescriptor(deviceDescriptor);
    server->audioPolicyService_.AddAudioCapturerMicrophoneDescriptor(sessionId, devType);
    server->audioPolicyService_.UpdateAudioCapturerMicrophoneDescriptor(devType);
    server->audioPolicyService_.RemoveAudioCapturerMicrophoneDescriptor(sessionId);
    EXPECT_EQ(devType, DeviceType::DEVICE_TYPE_NONE);
}

#ifdef BLUE_YELLOW_DIFF
/**
 * @tc.name  : Test GetTargetSourceTypeAndMatchingFlag.
 * @tc.number: GetTargetSourceTypeAndMatchingFlag_001
 * @tc.desc  : Test GetTargetSourceTypeAndMatchingFlag interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetTargetSourceTypeAndMatchingFlag_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    SourceType source = SourceType::SOURCE_TYPE_VOICE_CALL;
    SourceType targetSource;
    bool useMatchingPropInfo = true;

    server->audioPolicyService_.GetTargetSourceTypeAndMatchingFlag(source, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SourceType::SOURCE_TYPE_VOICE_CALL);

    source = SourceType::SOURCE_TYPE_CAMCORDER;
    server->audioPolicyService_.GetTargetSourceTypeAndMatchingFlag(source, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SourceType::SOURCE_TYPE_CAMCORDER);
}

/**
 * @tc.name  : Test GetEcType.
 * @tc.number: GetEcType_001
 * @tc.desc  : Test GetEcType interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetEcType_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    DeviceType inputDevice = DeviceType::DEVICE_TYPE_MIC;
    DeviceType outputDevice = DeviceType::DEVICE_TYPE_SPEAKER;

    EcType ecType = server->audioPolicyService_.GetEcType(inputDevice, outputDevice);
    EXPECT_EQ(ecType, EcType::EC_TYPE_SAME_ADAPTER);

    inputDevice = DeviceType::DEVICE_TYPE_MIC;
    outputDevice = DeviceType::DEVICE_TYPE_MIC;
    ecType = server->audioPolicyService_.GetEcType(inputDevice, outputDevice);
    EXPECT_EQ(ecType, EcType::EC_TYPE_NONE);
}

/**
 * @tc.name  : Test GetHalNameForDevice.
 * @tc.number: GetHalNameForDevice_001
 * @tc.desc  : Test GetHalNameForDevice interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetHalNameForDevice_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string role = ROLE_SINK;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    std::string halNameForDevice = server->audioPolicyService_.GetHalNameForDevice(role, deviceType);
    EXPECT_EQ(halNameForDevice, PRIMARY_CLASS);

    role = ROLE_SOURCE;
    halNameForDevice = server->audioPolicyService_.GetHalNameForDevice(role, deviceType);
    EXPECT_EQ(halNameForDevice, PRIMARY_CLASS);
}

/**
 * @tc.name  : Test GetPipeNameByDeviceForEc.
 * @tc.number: GetPipeNameByDeviceForEc_001
 * @tc.desc  : Test GetPipeNameByDeviceForEc interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetPipeNameByDeviceForEc_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string role = ROLE_SINK;
    DeviceType deviceType;
    std::string pipeNameByDeviceForEc;

    deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    pipeNameByDeviceForEc = server->audioPolicyService_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_PRIMARY_OUTPUT_UNITTEST);

    deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    pipeNameByDeviceForEc = server->audioPolicyService_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_PRIMARY_OUTPUT_UNITTEST);

    deviceType = DeviceType::DEVICE_TYPE_MIC;
    pipeNameByDeviceForEc = server->audioPolicyService_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_PRIMARY_INPUT_UNITTEST);

    deviceType = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;
    pipeNameByDeviceForEc = server->audioPolicyService_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_USB_ARM_OUTPUT_UNITTEST);

    deviceType = DeviceType::DEVICE_TYPE_DP;
    pipeNameByDeviceForEc = server->audioPolicyService_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_DP_OUTPUT_UNITTEST);

    deviceType = DeviceType::DEVICE_TYPE_MAX;
    pipeNameByDeviceForEc = server->audioPolicyService_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_PRIMARY_OUTPUT_UNITTEST);
    
    role = ROLE_SOURCE;
    deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    pipeNameByDeviceForEc = server->audioPolicyService_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_PRIMARY_INPUT_UNITTEST);

    deviceType = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;
    pipeNameByDeviceForEc = server->audioPolicyService_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_USB_ARM_INPUT_UNITTEST);
}

/**
 * @tc.name  : Test GetPipeInfoByDeviceTypeForEc.
 * @tc.number: GetPipeInfoByDeviceTypeForEc_001
 * @tc.desc  : Test GetPipeInfoByDeviceTypeForEc interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetPipeInfoByDeviceTypeForEc_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string role = ROLE_SOURCE;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    PipeInfo pipeInfo;

    int32_t ret = server->audioPolicyService_.GetPipeInfoByDeviceTypeForEc(role, deviceType, pipeInfo);
    EXPECT_EQ(ret, SUCCESS);

    role = ROLE_SINK;
    ret = server->audioPolicyService_.GetPipeInfoByDeviceTypeForEc(role, deviceType, pipeInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetAudioModuleInfoByName.
 * @tc.number: GetAudioModuleInfoByName_001
 * @tc.desc  : Test GetAudioModuleInfoByName interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetAudioModuleInfoByName_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    AudioEcInfo currentEcInfo = server->audioPolicyService_.GetAudioEcInfo();
    AudioModuleInfo currentModule;
    PipeInfo currentPipeInfo;

    int32_t currentModuleRet = server->audioPolicyService_.GetAudioModuleInfoByName(
        currentEcInfo.ecInputAdapter, currentPipeInfo.moduleName_, currentModule);
    EXPECT_EQ(currentModuleRet, ERROR);
}

/**
 * @tc.name  : Test ReloadSourceModuleForEc.
 * @tc.number: ReloadSourceModuleForEc_001
 * @tc.desc  : Test ReloadSourceModuleForEc interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, ReloadSourceModuleForEc_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    DeviceType inputDevice = DeviceType::DEVICE_TYPE_DEFAULT;
    DeviceType outputDevice = DeviceType::DEVICE_TYPE_DEFAULT;
    bool isForceReload = false;

    server->audioPolicyService_.isEcFeatureEnable_ = true;
    server->audioPolicyService_.audioScene_ = AudioScene::AUDIO_SCENE_PHONE_CHAT;
    server->audioPolicyService_.normalSourceOpened_ = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    server->audioPolicyService_.ReloadSourceModuleForEc(inputDevice, outputDevice, isForceReload);
    
    inputDevice = DeviceType::DEVICE_TYPE_MIC;
    outputDevice = DeviceType::DEVICE_TYPE_SPEAKER;
    server->audioPolicyService_.audioEcInfo_.inputDevice = DeviceType::DEVICE_TYPE_MIC;
    server->audioPolicyService_.audioEcInfo_.outputDevice = DeviceType::DEVICE_TYPE_SPEAKER;
    server->audioPolicyService_.ReloadSourceModuleForEc(inputDevice, outputDevice, isForceReload);

    server->audioPolicyService_.audioEcInfo_.inputDevice = DeviceType::DEVICE_TYPE_USB_HEADSET;
    server->audioPolicyService_.audioEcInfo_.outputDevice = DeviceType::DEVICE_TYPE_SPEAKER;
    server->audioPolicyService_.ReloadSourceModuleForEc(inputDevice, outputDevice, isForceReload);

    server->audioPolicyService_.audioEcInfo_.inputDevice = DeviceType::DEVICE_TYPE_MIC;
    server->audioPolicyService_.audioEcInfo_.outputDevice = DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    server->audioPolicyService_.ReloadSourceModuleForEc(inputDevice, outputDevice, isForceReload);

    server->audioPolicyService_.audioEcInfo_.inputDevice = DeviceType::DEVICE_TYPE_MIC;
    server->audioPolicyService_.audioEcInfo_.outputDevice = DeviceType::DEVICE_TYPE_SPEAKER;
    isForceReload = true;
    server->audioPolicyService_.ReloadSourceModuleForEc(inputDevice, outputDevice, isForceReload);

    server->audioPolicyService_.normalSourceOpened_ = SourceType::SOURCE_TYPE_MAX;
    server->audioPolicyService_.ReloadSourceModuleForEc(inputDevice, outputDevice, isForceReload);

    server->audioPolicyService_.audioScene_ = AudioScene::AUDIO_SCENE_DEFAULT;
    server->audioPolicyService_.ReloadSourceModuleForEc(inputDevice, outputDevice, isForceReload);

    server->audioPolicyService_.isEcFeatureEnable_ = false;
    server->audioPolicyService_.ReloadSourceModuleForEc(inputDevice, outputDevice, isForceReload);
    EXPECT_EQ(isForceReload, true);
}

/**
 * @tc.name  : Test GetEcSamplingRate.
 * @tc.number: GetEcSamplingRate_001
 * @tc.desc  : Test GetEcSamplingRate interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetEcSamplingRate_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string halName;
    StreamPropInfo outModuleInfo;

    halName = DP_CLASS;
    std::string ecSamplingRate = server->audioPolicyService_.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(ecSamplingRate, "0");

    halName = USB_CLASS;
    ecSamplingRate = server->audioPolicyService_.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(ecSamplingRate, "0");

    halName = INVALID_CLASS;
    ecSamplingRate = server->audioPolicyService_.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(ecSamplingRate, "");
}

/**
 * @tc.name  : Test GetEcFormat.
 * @tc.number: GetEcFormat_001
 * @tc.desc  : Test GetEcFormat interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetEcFormat_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string halName;
    StreamPropInfo outModuleInfo;

    halName = DP_CLASS;
    std::string ecFormat = server->audioPolicyService_.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(ecFormat, "");

    halName = USB_CLASS;
    ecFormat = server->audioPolicyService_.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(ecFormat, "");

    halName = INVALID_CLASS;
    ecFormat = server->audioPolicyService_.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(ecFormat, "");
}

/**
 * @tc.name  : Test GetEcChannels.
 * @tc.number: GetEcChannels_001
 * @tc.desc  : Test GetEcChannels interfaces.
 */
HWTEST(AudioPolicyServiceExtUnitTest, GetEcChannels_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    std::string halName;
    std::string ecChannels;
    StreamPropInfo outModuleInfo;

    halName = DP_CLASS;
    ecChannels = server->audioPolicyService_.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(ecChannels, "0");

    halName = USB_CLASS;
    ecChannels = server->audioPolicyService_.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(ecChannels, "0");

    halName = INVALID_CLASS;
    ecChannels = server->audioPolicyService_.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(ecChannels, "4");
}
#endif
} // namespace AudioStandard
} // namespace OHOS