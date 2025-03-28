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
#include "get_server_util.h"
#include "audio_policy_service_second_unit_test.h"
#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_adapter_info.h"
#include "audio_module_info.h"
#include "audio_ec_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_policy_manager.h"
#include "audio_session_info.h"
#include "audio_system_manager.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include <memory>
#include <thread>
#include <vector>

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace AudioStandard {
static bool g_hasPermission = false;
static const std::string PIPE_PRIMARY_OUTPUT_UNITTEST = "primary_output";
static const std::string PIPE_PRIMARY_INPUT_UNITTEST = "primary_input";
static const std::string PIPE_USB_ARM_OUTPUT_UNITTEST = "usb_arm_output";
static const std::string PIPE_DP_OUTPUT_UNITTEST = "dp_output";
static const std::string PIPE_USB_ARM_INPUT_UNITTEST = "usb_arm_input";

void AudioPolicyServiceExtUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtUnitTest::SetUpTestCase start-end");
}
void AudioPolicyServiceExtUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtUnitTest::TearDownTestCase start-end");
}
void AudioPolicyServiceExtUnitTest::SetUp(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtUnitTest::SetUp start-end");
}
void AudioPolicyServiceExtUnitTest::TearDown(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceExtUnitTest::TearDown start-end");
}

static void GetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 24;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
            "ohos.permission.ACCESS_DISTRIBUTED_HARDWARE",
            "ohos.permission.REPORT_RESOURCE_SCHEDULE_EVENT",
            "ohos.permission.GET_SENSITIVE_PERMISSIONS",
            "ohos.permission.PERMISSION_USED_STATS",
            "ohos.permission.ACCESS_SERVICE_DM",
            "ohos.permission.MONITOR_DEVICE_NETWORK_STATE",
            "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED",
            "ohos.permission.MANAGE_SECURE_SETTINGS",
            "ohos.permission.MANAGE_SETTINGS",
            "ohos.permission.ACCESS_BLUETOOTH",
            "ohos.permission.MANAGE_BLUETOOTH",
            "ohos.permission.MANAGE_LOCAL_ACCOUNTS",
            "ohos.permission.DISTRIBUTED_DATASYNC",
            "ohos.permission.MODIFY_AUDIO_SET",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 24,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audio_policy_service_ext_unit_test",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

/**
 * @tc.name  : Test AudioVolumeDump.
 * @tc.number: AudioVolumeDump_001
 * @tc.desc  : Test AudioVolumeDump interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioVolumeDump_001, TestSize.Level1)
{
    GetPermission();
    auto server = GetServerUtil::GetServerPtr();
    std::string dumpString = "666";
    server->AudioVolumeDump(dumpString);
    EXPECT_NE(dumpString, "666");
}

/**
 * @tc.name  : Test AudioStreamDump.
 * @tc.number: AudioStreamDump_001
 * @tc.desc  : Test AudioStreamDump interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioStreamDump_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string dumpString = "666";
    server->AudioStreamDump(dumpString);
    EXPECT_NE(dumpString, "666");
}

/**
 * @tc.name  : Test CheckAudioSessionStrategy.
 * @tc.number: CheckAudioSessionStrategy_001
 * @tc.desc  : Test CheckAudioSessionStrategy interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, CheckAudioSessionStrategy_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioSessionStrategy sessionStrategy;
    sessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    bool ret = server->CheckAudioSessionStrategy(sessionStrategy);
    EXPECT_EQ(ret, true);

    sessionStrategy.concurrencyMode = (AudioConcurrencyMode)666;
    ret = server->CheckAudioSessionStrategy(sessionStrategy);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test HandleA2dpDeviceInOffload.
 * @tc.number: HandleA2dpDeviceInOffload_001
 * @tc.desc  : Test HandleA2dpDeviceInOffload interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, HandleA2dpDeviceInOffload_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    BluetoothOffloadState a2dpOffloadFlag = A2DP_NOT_OFFLOAD;
    int32_t ret = server->audioPolicyService_.audioA2dpOffloadManager_->HandleA2dpDeviceInOffload(a2dpOffloadFlag);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ReconfigureAudioChannel.
 * @tc.number: ReconfigureAudioChannel_001
 * @tc.desc  : Test ReconfigureAudioChannel interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, ReconfigureAudioChannel_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    uint32_t channelCount = 1;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    int32_t ret = server->audioPolicyService_.ReconfigureAudioChannel(channelCount, deviceType);
    EXPECT_EQ(ret, ERROR);

    deviceType = DeviceType::DEVICE_TYPE_FILE_SINK;
    ret = server->audioPolicyService_.ReconfigureAudioChannel(channelCount, deviceType);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test WriteAllDeviceSysEvents.
 * @tc.number: WriteAllDeviceSysEvents_001
 * @tc.desc  : Test WriteAllDeviceSysEvents interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, WriteAllDeviceSysEvents_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    bool isConnected;

    isConnected = false;
    desc = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::ALL_DEVICES_FLAG);
    server->audioPolicyService_.audioDeviceStatus_.WriteAllDeviceSysEvents(desc, isConnected);
    EXPECT_EQ(isConnected, false);
}

/**
 * @tc.name  : Test UpdateTrackerDeviceChange.
 * @tc.number: UpdateTrackerDeviceChange_001
 * @tc.desc  : Test UpdateTrackerDeviceChange interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, UpdateTrackerDeviceChange_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc =
        AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::ALL_DEVICES_FLAG);
    server->audioPolicyService_.audioDeviceLock_.UpdateTrackerDeviceChange(desc);
    EXPECT_TRUE(desc.size() >= 0);
}

/**
 * @tc.name  : Test SetAbsVolumeSceneAsync.
 * @tc.number: SetAbsVolumeSceneAsync_001
 * @tc.desc  : Test SetAbsVolumeSceneAsync interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, SetAbsVolumeSceneAsync_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string macAddress = "";
    bool support = false;
    server->audioPolicyService_.audioVolumeManager_.SetAbsVolumeSceneAsync(macAddress, support);
    EXPECT_EQ(server->audioPolicyService_.audioActiveDevice_.activeBTDevice_, "");
}

/**
 * @tc.name  : Test SetDeviceAbsVolumeSupported.
 * @tc.number: SetDeviceAbsVolumeSupported_001
 * @tc.desc  : Test SetDeviceAbsVolumeSupported interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, SetDeviceAbsVolumeSupported_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
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
HWTEST_F(AudioPolicyServiceExtUnitTest, IsWiredHeadSet_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    DeviceType deviceType = DeviceType::DEVICE_TYPE_MAX;
    bool ret = server->audioPolicyService_.audioVolumeManager_.IsWiredHeadSet(deviceType);
    EXPECT_EQ(ret, false);

    deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    ret = server->audioPolicyService_.audioVolumeManager_.IsWiredHeadSet(deviceType);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsBlueTooth.
 * @tc.number: IsBlueTooth_001
 * @tc.desc  : Test IsBlueTooth interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, IsBlueTooth_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    bool ret = server->audioPolicyService_.audioVolumeManager_.IsBlueTooth(deviceType);
    EXPECT_EQ(ret, false);

    deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = server->audioPolicyService_.audioVolumeManager_.IsBlueTooth(deviceType);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test CheckBlueToothActiveMusicTime.
 * @tc.number: CheckBlueToothActiveMusicTime_001
 * @tc.desc  : Test CheckBlueToothActiveMusicTime interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, CheckBlueToothActiveMusicTime_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    int32_t safeVolume = 1;
    server->audioPolicyService_.audioVolumeManager_.CheckBlueToothActiveMusicTime(safeVolume);
    EXPECT_EQ(server->audioPolicyService_.audioVolumeManager_.startSafeTime_, 0);
}

/**
 * @tc.name  : Test CheckWiredActiveMusicTime.
 * @tc.number: CheckWiredActiveMusicTime_001
 * @tc.desc  : Test CheckWiredActiveMusicTime interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, CheckWiredActiveMusicTime_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    int32_t safeVolume = 1;
    server->audioPolicyService_.audioVolumeManager_.CheckWiredActiveMusicTime(safeVolume);
    EXPECT_EQ(server->audioPolicyService_.audioVolumeManager_.startSafeTimeBt_, 0);
}

/**
 * @tc.name  : Test RestoreSafeVolume.
 * @tc.number: RestoreSafeVolume_001
 * @tc.desc  : Test RestoreSafeVolume interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, RestoreSafeVolume_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioStreamType streamType = AudioStreamType::STREAM_RING;
    int32_t safeVolume = 1;
    server->audioPolicyService_.audioVolumeManager_.RestoreSafeVolume(streamType, safeVolume);
    EXPECT_EQ(server->audioPolicyService_.audioVolumeManager_.userSelect_, false);
}

/**
 * @tc.name  : Test CreateCheckMusicActiveThread.
 * @tc.number: CreateCheckMusicActiveThread_001
 * @tc.desc  : Test CreateCheckMusicActiveThread interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, CreateCheckMusicActiveThread_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    server->audioPolicyService_.audioVolumeManager_.CreateCheckMusicActiveThread();
    EXPECT_NE(server->audioPolicyService_.audioVolumeManager_.calculateLoopSafeTime_, nullptr);
}

/**
 * @tc.name  : Test DealWithSafeVolume.
 * @tc.number: DealWithSafeVolume_001
 * @tc.desc  : Test DealWithSafeVolume interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, DealWithSafeVolume_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    int32_t volumeLevel = 8;
    bool isA2dpDevice = true;
    int32_t volumeLevelRet
        = server->audioPolicyService_.audioVolumeManager_.DealWithSafeVolume(volumeLevel, isA2dpDevice);
    EXPECT_EQ(volumeLevelRet, 15);

    isA2dpDevice = false;
    volumeLevelRet = server->audioPolicyService_.audioVolumeManager_.DealWithSafeVolume(volumeLevel, isA2dpDevice);
    EXPECT_EQ(volumeLevelRet, 15);

    volumeLevel = 11;
    volumeLevelRet = server->audioPolicyService_.audioVolumeManager_.DealWithSafeVolume(volumeLevel, isA2dpDevice);
    EXPECT_EQ(volumeLevelRet, 15);
}

/**
 * @tc.name  : Test HandleAbsBluetoothVolume.
 * @tc.number: HandleAbsBluetoothVolume_001
 * @tc.desc  : Test HandleAbsBluetoothVolume interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, HandleAbsBluetoothVolume_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string macAddress = "";
    int32_t volumeLevel = 10;
    int32_t safeVolumeLevel
        = server->audioPolicyService_.audioVolumeManager_.HandleAbsBluetoothVolume(macAddress, volumeLevel);
    EXPECT_EQ(safeVolumeLevel, 15);

    volumeLevel = 8;
    safeVolumeLevel
        = server->audioPolicyService_.audioVolumeManager_.HandleAbsBluetoothVolume(macAddress, volumeLevel);
    EXPECT_EQ(safeVolumeLevel, 15);
}

/**
 * @tc.name  : Test SetA2dpDeviceVolume.
 * @tc.number: SetA2dpDeviceVolume_001
 * @tc.desc  : Test SetA2dpDeviceVolume interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, SetA2dpDeviceVolume_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string macAddress = "";
    int32_t volumeLevel = 4;
    bool internalCall = false;
    int32_t ret = server->audioPolicyService_.SetA2dpDeviceVolume(macAddress, volumeLevel, internalCall);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test SetA2dpDeviceVolume.
 * @tc.number: SetA2dpDeviceVolume_002
 * @tc.desc  : Test SetA2dpDeviceVolume interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, SetA2dpDeviceVolume_002, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string macAddress = "A1:B2:C3:D4:E5:F6";
    A2dpDeviceConfigInfo configInfo{};
    configInfo.absVolumeSupport = true;
    server->audioPolicyService_.audioDeviceStatus_.audioA2dpDevice_.AddA2dpDevice(macAddress, configInfo);
    int32_t volumeLevel = 4;
    bool internalCall = true;
    int32_t ret = server->audioPolicyService_.SetA2dpDeviceVolume(macAddress, volumeLevel, internalCall);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test TriggerDeviceChangedCallback.
 * @tc.number: TriggerDeviceChangedCallback_001
 * @tc.desc  : Test TriggerDeviceChangedCallback interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, TriggerDeviceChangedCallback_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    vector<std::shared_ptr<AudioDeviceDescriptor>> desc =
        AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::ALL_DEVICES_FLAG);
    bool isConnected = false;
    server->audioPolicyService_.audioDeviceStatus_.TriggerDeviceChangedCallback(desc, isConnected);
    EXPECT_NE(server->audioPolicyService_.audioPolicyServerHandler_, nullptr);
}

/**
 * @tc.name  : Test GetDeviceRole.
 * @tc.number: GetDeviceRole_001
 * @tc.desc  : Test GetDeviceRole interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetDeviceRole_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    DeviceRole deviceRole = AudioPolicyUtils::GetInstance().GetDeviceRole(deviceType);
    EXPECT_EQ(deviceRole, DeviceRole::OUTPUT_DEVICE);

    deviceType = DeviceType::DEVICE_TYPE_WAKEUP;
    deviceRole = AudioPolicyUtils::GetInstance().GetDeviceRole(deviceType);
    EXPECT_EQ(deviceRole, DeviceRole::INPUT_DEVICE);

    deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
    deviceRole = AudioPolicyUtils::GetInstance().GetDeviceRole(deviceType);
    EXPECT_EQ(deviceRole, DeviceRole::DEVICE_ROLE_NONE);
}

/**
 * @tc.name  : Test GetDeviceRole.
 * @tc.number: GetDeviceRole_002
 * @tc.desc  : Test GetDeviceRole interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetDeviceRole_002, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioPin pin = AudioPin::AUDIO_PIN_NONE;
    DeviceRole deviceRole = AudioPolicyUtils::GetInstance().GetDeviceRole(pin);
    EXPECT_EQ(deviceRole, DeviceRole::DEVICE_ROLE_NONE);

    pin = AudioPin::AUDIO_PIN_OUT_SPEAKER;
    deviceRole = AudioPolicyUtils::GetInstance().GetDeviceRole(pin);
    EXPECT_EQ(deviceRole, DeviceRole::OUTPUT_DEVICE);

    pin = AudioPin::AUDIO_PIN_IN_MIC;
    deviceRole = AudioPolicyUtils::GetInstance().GetDeviceRole(pin);
    EXPECT_EQ(deviceRole, DeviceRole::INPUT_DEVICE);

    pin = (AudioPin)666;
    deviceRole = AudioPolicyUtils::GetInstance().GetDeviceRole(pin);
    EXPECT_EQ(deviceRole, DeviceRole::DEVICE_ROLE_NONE);
}

/**
 * @tc.name  : Test GetVoipRendererFlag.
 * @tc.number: GetVoipRendererFlag_001
 * @tc.desc  : Test GetVoipRendererFlag interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetVoipRendererFlag_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string sinkPortName = "";
    std::string networkId = "";
    AudioSamplingRate samplingRate = SAMPLE_RATE_8000;
    int32_t ret;

    server->audioPolicyService_.audioConfigManager_.normalVoipFlag_ = false;
    server->audioPolicyService_.audioConfigManager_.OnVoipConfigParsed(false);
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    networkId = LOCAL_NETWORK_ID;
    server->audioPolicyService_.audioConfigManager_.normalVoipFlag_ = true;
    server->audioPolicyService_.audioConfigManager_.OnVoipConfigParsed(true);
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    sinkPortName = PRIMARY_SPEAKER;
    networkId = REMOTE_NETWORK_ID;
    server->audioPolicyService_.audioConfigManager_.OnVoipConfigParsed(false);
    server->audioPolicyService_.audioConfigManager_.normalVoipFlag_ = false;
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    sinkPortName = BLUETOOTH_SPEAKER;
    networkId = REMOTE_NETWORK_ID;
    server->audioPolicyService_.audioConfigManager_.OnVoipConfigParsed(false);
    server->audioPolicyService_.audioConfigManager_.normalVoipFlag_ = false;
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    sinkPortName = PRIMARY_SPEAKER;
    networkId = LOCAL_NETWORK_ID;
    server->audioPolicyService_.audioConfigManager_.OnVoipConfigParsed(true);
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    sinkPortName = USB_SPEAKER;
    networkId = REMOTE_NETWORK_ID;
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    sinkPortName = PRIMARY_SPEAKER;
    networkId = LOCAL_NETWORK_ID;
    server->audioPolicyService_.audioConfigManager_.OnVoipConfigParsed(false);
    server->audioPolicyService_.audioConfigManager_.normalVoipFlag_ = false;
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_VOIP_DIRECT);
}

/**
 * @tc.name  : Test GetVoipRendererFlag.
 * @tc.number: GetVoipRendererFlag_002
 * @tc.desc  : Test GetVoipRendererFlag interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetVoipRendererFlag_002, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string sinkPortName = "";
    std::string networkId = "";
    AudioSamplingRate samplingRate = SAMPLE_RATE_16000;
    int32_t ret;

    sinkPortName = PRIMARY_SPEAKER;
    networkId = LOCAL_NETWORK_ID;
    server->audioPolicyService_.audioConfigManager_.OnVoipConfigParsed(true);
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_VOIP_FAST);

    sinkPortName = USB_SPEAKER;
    networkId = REMOTE_NETWORK_ID;
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    samplingRate = SAMPLE_RATE_48000;
    sinkPortName = PRIMARY_SPEAKER;
    networkId = LOCAL_NETWORK_ID;
    server->audioPolicyService_.audioConfigManager_.OnVoipConfigParsed(true);
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_VOIP_FAST);

    sinkPortName = USB_SPEAKER;
    networkId = REMOTE_NETWORK_ID;
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    samplingRate = SAMPLE_RATE_44100;
    sinkPortName = PRIMARY_SPEAKER;
    networkId = LOCAL_NETWORK_ID;
    server->audioPolicyService_.audioConfigManager_.OnVoipConfigParsed(true);
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);

    sinkPortName = USB_SPEAKER;
    networkId = REMOTE_NETWORK_ID;
    ret = server->audioPolicyService_.audioConfigManager_.GetVoipRendererFlag(sinkPortName, networkId, samplingRate);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);
}

/**
 * @tc.name  : Test UpdateInputDeviceInfo.
 * @tc.number: UpdateInputDeviceInfo_001
 * @tc.desc  : Test UpdateInputDeviceInfo interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, UpdateInputDeviceInfo_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    server->audioPolicyService_.audioActiveDevice_.UpdateInputDeviceInfo(deviceType);
    EXPECT_EQ(server->audioPolicyService_.audioActiveDevice_.currentActiveInputDevice_.deviceType_,
        DeviceType::DEVICE_TYPE_MIC);

    deviceType = DeviceType::DEVICE_TYPE_FILE_SINK;
    server->audioPolicyService_.audioActiveDevice_.UpdateInputDeviceInfo(deviceType);
    EXPECT_EQ(server->audioPolicyService_.audioActiveDevice_.currentActiveInputDevice_.deviceType_,
        DeviceType::DEVICE_TYPE_FILE_SOURCE);

    deviceType = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;
    server->audioPolicyService_.audioActiveDevice_.UpdateInputDeviceInfo(deviceType);
    EXPECT_EQ(server->audioPolicyService_.audioActiveDevice_.currentActiveInputDevice_.deviceType_,
        DeviceType::DEVICE_TYPE_USB_HEADSET);

    deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    server->audioPolicyService_.audioActiveDevice_.UpdateInputDeviceInfo(deviceType);
    EXPECT_EQ(server->audioPolicyService_.audioActiveDevice_.currentActiveInputDevice_.deviceType_,
        DeviceType::DEVICE_TYPE_WIRED_HEADSET);

    deviceType = (DeviceType)777;
    server->audioPolicyService_.audioActiveDevice_.UpdateInputDeviceInfo(deviceType);
    EXPECT_EQ(server->audioPolicyService_.audioActiveDevice_.currentActiveInputDevice_.deviceType_,
        DeviceType::DEVICE_TYPE_WIRED_HEADSET);
}


/**
 * @tc.name  : Test GetDeviceTypeFromPin.
 * @tc.number: GetDeviceTypeFromPin_001
 * @tc.desc  : Test GetDeviceTypeFromPin interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetDeviceTypeFromPin_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioPin hdiPin;
    hdiPin = AudioPin::AUDIO_PIN_NONE;
    DeviceType deviceType = server->audioPolicyService_.audioDeviceStatus_.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DeviceType::DEVICE_TYPE_DEFAULT);

    hdiPin = AudioPin::AUDIO_PIN_OUT_SPEAKER;
    deviceType = server->audioPolicyService_.audioDeviceStatus_.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DeviceType::DEVICE_TYPE_SPEAKER);

    hdiPin = AudioPin::AUDIO_PIN_OUT_USB_HEADSET;
    deviceType = server->audioPolicyService_.audioDeviceStatus_.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DeviceType::DEVICE_TYPE_USB_ARM_HEADSET);

    hdiPin = AudioPin::AUDIO_PIN_IN_MIC;
    deviceType = server->audioPolicyService_.audioDeviceStatus_.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DeviceType::DEVICE_TYPE_MIC);

    hdiPin = (AudioPin)666;
    deviceType = server->audioPolicyService_.audioDeviceStatus_.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DeviceType::DEVICE_TYPE_DEFAULT);
}

/**
 * @tc.name  : Test GetProcessDeviceInfo.
 * @tc.number: GetProcessDeviceInfo_001
 * @tc.desc  : Test GetProcessDeviceInfo interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetProcessDeviceInfo_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioProcessConfig config;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    int32_t ret;

    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    ret = server->audioPolicyService_.GetProcessDeviceInfo(config, true, deviceInfo);
    EXPECT_EQ(ret, ERROR);

    config.rendererInfo.streamUsage = STREAM_USAGE_VIDEO_COMMUNICATION;
    ret = server->audioPolicyService_.GetProcessDeviceInfo(config, true, deviceInfo);
    EXPECT_EQ(ret, ERROR);

    config.rendererInfo.streamUsage = STREAM_USAGE_UNKNOWN;
    ret = server->audioPolicyService_.GetProcessDeviceInfo(config, true, deviceInfo);
    EXPECT_EQ(ret, SUCCESS);

    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    ret = server->audioPolicyService_.GetProcessDeviceInfo(config, true, deviceInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetVoipDeviceInfo.
 * @tc.number: GetVoipDeviceInfo_001
 * @tc.desc  : Test GetVoipDeviceInfo interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetVoipDeviceInfo_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioRendererInfo rendererInfo;
    AudioProcessConfig config;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
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
HWTEST_F(AudioPolicyServiceExtUnitTest, GetSharedVolume_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioVolumeType streamType = AudioStreamType::STREAM_RING;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    Volume vol;

    bool ret = server->audioPolicyService_.GetSharedVolume(streamType, deviceType, vol);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test UpdateAudioCapturerMicrophoneDescriptor.
 * @tc.number: UpdateAudioCapturerMicrophoneDescriptor_001
 * @tc.desc  : Test UpdateAudioCapturerMicrophoneDescriptor interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, UpdateAudioCapturerMicrophoneDescriptor_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    DeviceType devType = DeviceType::DEVICE_TYPE_NONE;
    int32_t sessionId = 0;
    vector<sptr<MicrophoneDescriptor>> AudioCapturerMicrophoneDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();

    server->audioPolicyService_.audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(sessionId, devType);
    server->audioPolicyService_.audioMicrophoneDescriptor_.AddMicrophoneDescriptor(deviceDescriptor);
    server->audioPolicyService_.audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(sessionId, devType);
    server->audioPolicyService_.audioMicrophoneDescriptor_.UpdateAudioCapturerMicrophoneDescriptor(devType);
    AudioCapturerMicrophoneDescriptors = server->audioPolicyService_.GetAudioCapturerMicrophoneDescriptors(sessionId);
    server->audioPolicyService_.audioMicrophoneDescriptor_.RemoveMicrophoneDescriptor(deviceDescriptor);
    server->audioPolicyService_.audioMicrophoneDescriptor_.RemoveAudioCapturerMicrophoneDescriptor(sessionId);
    EXPECT_TRUE(server->audioPolicyService_.audioMicrophoneDescriptor_.connectedMicrophones_.size() >= 0);
}

/**
 * @tc.name  : Test GetTargetSourceTypeAndMatchingFlag.
 * @tc.number: GetTargetSourceTypeAndMatchingFlag_001
 * @tc.desc  : Test GetTargetSourceTypeAndMatchingFlag interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetTargetSourceTypeAndMatchingFlag_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    SourceType source = SourceType::SOURCE_TYPE_VOICE_CALL;
    SourceType targetSource;
    bool useMatchingPropInfo = true;

    server->audioPolicyService_.audioEcManager_.GetTargetSourceTypeAndMatchingFlag(source, targetSource,
        useMatchingPropInfo);
    EXPECT_EQ(targetSource, SourceType::SOURCE_TYPE_VOICE_CALL);

    source = SourceType::SOURCE_TYPE_CAMCORDER;
    server->audioPolicyService_.audioEcManager_.GetTargetSourceTypeAndMatchingFlag(source, targetSource,
        useMatchingPropInfo);
    EXPECT_EQ(targetSource, SourceType::SOURCE_TYPE_MIC);
}

/**
 * @tc.name  : Test GetEcType.
 * @tc.number: GetEcType_001
 * @tc.desc  : Test GetEcType interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetEcType_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    DeviceType inputDevice = DeviceType::DEVICE_TYPE_MIC;
    DeviceType outputDevice = DeviceType::DEVICE_TYPE_SPEAKER;

    EcType ecType = server->audioPolicyService_.audioEcManager_.GetEcType(inputDevice, outputDevice);
    EXPECT_EQ(ecType, EcType::EC_TYPE_SAME_ADAPTER);

    inputDevice = DeviceType::DEVICE_TYPE_MIC;
    outputDevice = DeviceType::DEVICE_TYPE_MIC;
    ecType = server->audioPolicyService_.audioEcManager_.GetEcType(inputDevice, outputDevice);
    EXPECT_EQ(ecType, EcType::EC_TYPE_NONE);
}

/**
 * @tc.name  : Test GetHalNameForDevice.
 * @tc.number: GetHalNameForDevice_001
 * @tc.desc  : Test GetHalNameForDevice interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetHalNameForDevice_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string role = ROLE_SINK;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    std::string halNameForDevice = server->audioPolicyService_.audioEcManager_.GetHalNameForDevice(role, deviceType);
    EXPECT_EQ(halNameForDevice, "primary");

    role = ROLE_SOURCE;
    halNameForDevice = server->audioPolicyService_.audioEcManager_.GetHalNameForDevice(role, deviceType);
    EXPECT_EQ(halNameForDevice, "primary");
}

/**
 * @tc.name  : Test GetPipeNameByDeviceForEc.
 * @tc.number: GetPipeNameByDeviceForEc_001
 * @tc.desc  : Test GetPipeNameByDeviceForEc interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetPipeNameByDeviceForEc_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string role = ROLE_SINK;
    DeviceType deviceType;
    std::string pipeNameByDeviceForEc;

    deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    pipeNameByDeviceForEc = server->audioPolicyService_.audioEcManager_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_PRIMARY_OUTPUT_UNITTEST);

    deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    pipeNameByDeviceForEc = server->audioPolicyService_.audioEcManager_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_PRIMARY_OUTPUT_UNITTEST);

    deviceType = DeviceType::DEVICE_TYPE_MIC;
    pipeNameByDeviceForEc = server->audioPolicyService_.audioEcManager_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_PRIMARY_INPUT_UNITTEST);

    deviceType = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;
    pipeNameByDeviceForEc = server->audioPolicyService_.audioEcManager_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_USB_ARM_OUTPUT_UNITTEST);

    deviceType = DeviceType::DEVICE_TYPE_DP;
    pipeNameByDeviceForEc = server->audioPolicyService_.audioEcManager_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_DP_OUTPUT_UNITTEST);

    deviceType = DeviceType::DEVICE_TYPE_MAX;
    pipeNameByDeviceForEc = server->audioPolicyService_.audioEcManager_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_PRIMARY_OUTPUT_UNITTEST);

    role = ROLE_SOURCE;
    deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    pipeNameByDeviceForEc = server->audioPolicyService_.audioEcManager_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_PRIMARY_INPUT_UNITTEST);

    deviceType = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;
    pipeNameByDeviceForEc = server->audioPolicyService_.audioEcManager_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_USB_ARM_INPUT_UNITTEST);
}

/**
 * @tc.name  : Test GetPipeInfoByDeviceTypeForEc.
 * @tc.number: GetPipeInfoByDeviceTypeForEc_001
 * @tc.desc  : Test GetPipeInfoByDeviceTypeForEc interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetPipeInfoByDeviceTypeForEc_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string role = ROLE_SOURCE;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    PipeInfo pipeInfo;

    int32_t ret = server->audioPolicyService_.audioEcManager_.GetPipeInfoByDeviceTypeForEc(role, deviceType, pipeInfo);
    EXPECT_EQ(ret, SUCCESS);

    role = ROLE_SINK;
    ret = server->audioPolicyService_.audioEcManager_.GetPipeInfoByDeviceTypeForEc(role, deviceType, pipeInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetEcSamplingRate.
 * @tc.number: GetEcSamplingRate_001
 * @tc.desc  : Test GetEcSamplingRate interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetEcSamplingRate_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string halName;
    StreamPropInfo outModuleInfo;
    std::string ecSamplingRate;

    halName = DP_CLASS;
    server->audioPolicyService_.audioEcManager_.dpSinkModuleInfo_.rate = "888";
    ecSamplingRate = server->audioPolicyService_.audioEcManager_.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(ecSamplingRate, "888");

    server->audioPolicyService_.audioEcManager_.dpSinkModuleInfo_.rate = "";
    ecSamplingRate = server->audioPolicyService_.audioEcManager_.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(ecSamplingRate, "0");

    halName = USB_CLASS;
    server->audioPolicyService_.audioEcManager_.usbSinkModuleInfo_.rate = "666";
    ecSamplingRate = server->audioPolicyService_.audioEcManager_.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(ecSamplingRate, "666");

    server->audioPolicyService_.audioEcManager_.usbSinkModuleInfo_.rate = "";
    ecSamplingRate = server->audioPolicyService_.audioEcManager_.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(ecSamplingRate, "0");

    halName = INVALID_CLASS;
    ecSamplingRate = server->audioPolicyService_.audioEcManager_.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(ecSamplingRate, "44100");
}

/**
 * @tc.name  : Test GetEcFormat.
 * @tc.number: GetEcFormat_001
 * @tc.desc  : Test GetEcFormat interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetEcFormat_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string halName;
    StreamPropInfo outModuleInfo;
    std::string ecFormat;

    halName = DP_CLASS;
    server->audioPolicyService_.audioEcManager_.dpSinkModuleInfo_.format = "666";
    ecFormat = server->audioPolicyService_.audioEcManager_.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(ecFormat, "666");

    server->audioPolicyService_.audioEcManager_.dpSinkModuleInfo_.format = "";
    ecFormat = server->audioPolicyService_.audioEcManager_.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(ecFormat, "");

    halName = USB_CLASS;
    server->audioPolicyService_.audioEcManager_.usbSinkModuleInfo_.format = "444";
    ecFormat = server->audioPolicyService_.audioEcManager_.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(ecFormat, "444");

    server->audioPolicyService_.audioEcManager_.usbSinkModuleInfo_.format = "";
    ecFormat = server->audioPolicyService_.audioEcManager_.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(ecFormat, "");

    halName = INVALID_CLASS;
    ecFormat = server->audioPolicyService_.audioEcManager_.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(ecFormat, "s16le");
}

/**
 * @tc.name  : Test GetEcChannels.
 * @tc.number: GetEcChannels_001
 * @tc.desc  : Test GetEcChannels interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetEcChannels_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::string halName;
    std::string ecChannels;
    StreamPropInfo outModuleInfo;

    halName = DP_CLASS;
    server->audioPolicyService_.audioEcManager_.dpSinkModuleInfo_.channels = "666";
    ecChannels = server->audioPolicyService_.audioEcManager_.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(ecChannels, "666");

    server->audioPolicyService_.audioEcManager_.dpSinkModuleInfo_.channels = "";
    ecChannels = server->audioPolicyService_.audioEcManager_.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(ecChannels, "0");

    halName = USB_CLASS;
    server->audioPolicyService_.audioEcManager_.usbSinkModuleInfo_.channels = "555";
    ecChannels = server->audioPolicyService_.audioEcManager_.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(ecChannels, "555");

    server->audioPolicyService_.audioEcManager_.usbSinkModuleInfo_.channels = "";
    ecChannels = server->audioPolicyService_.audioEcManager_.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(ecChannels, "0");

    halName = INVALID_CLASS;
    server->audioPolicyService_.audioEcManager_.audioEcInfo_.inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    ecChannels = server->audioPolicyService_.audioEcManager_.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(ecChannels, "2");

    server->audioPolicyService_.audioEcManager_.audioEcInfo_.inputDevice.deviceType_ = DEVICE_TYPE_MAX;
    ecChannels = server->audioPolicyService_.audioEcManager_.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(ecChannels, "2");
}

/**
 * @tc.name  : Test UpdateAudioEcInfo.
 * @tc.number: UpdateAudioEcInfo_001
 * @tc.desc  : Test UpdateAudioEcInfo interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, UpdateAudioEcInfo_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioDeviceDescriptor inputDevice, outputDevice;
    inputDevice.deviceType_ = DeviceType::DEVICE_TYPE_MAX;
    outputDevice.deviceType_ = DeviceType::DEVICE_TYPE_MAX;

    server->audioPolicyService_.audioEcManager_.isEcFeatureEnable_ = false;
    server->audioPolicyService_.audioEcManager_.UpdateAudioEcInfo(inputDevice, outputDevice);

    inputDevice = server->audioPolicyService_.audioEcManager_.audioEcInfo_.inputDevice;
    outputDevice = server->audioPolicyService_.audioEcManager_.audioEcInfo_.outputDevice;
    server->audioPolicyService_.audioEcManager_.isEcFeatureEnable_ = true;
    server->audioPolicyService_.audioEcManager_.UpdateAudioEcInfo(inputDevice, outputDevice);

    inputDevice = server->audioPolicyService_.audioEcManager_.audioEcInfo_.inputDevice;
    outputDevice.deviceType_ = DeviceType::DEVICE_TYPE_MAX;
    server->audioPolicyService_.audioEcManager_.UpdateAudioEcInfo(inputDevice, outputDevice);

    inputDevice.deviceType_ = DeviceType::DEVICE_TYPE_MAX;
    outputDevice = server->audioPolicyService_.audioEcManager_.audioEcInfo_.outputDevice;
    server->audioPolicyService_.audioEcManager_.UpdateAudioEcInfo(inputDevice, outputDevice);

    inputDevice.deviceType_ = DeviceType::DEVICE_TYPE_MAX;
    outputDevice.deviceType_ = DeviceType::DEVICE_TYPE_MAX;
    server->audioPolicyService_.audioEcManager_.UpdateAudioEcInfo(inputDevice, outputDevice);
    EXPECT_EQ(server->audioPolicyService_.audioEcManager_.isEcFeatureEnable_, true);
}

/**
 * @tc.name  : Test UpdateStreamCommonInfo.
 * @tc.number: UpdateStreamCommonInfo_001
 * @tc.desc  : Test UpdateStreamCommonInfo interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, UpdateStreamCommonInfo_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioModuleInfo moduleInfo;
    StreamPropInfo targetInfo;
    SourceType sourceType = SourceType::SOURCE_TYPE_MIC;

    server->audioPolicyService_.audioEcManager_.isEcFeatureEnable_ = true;
    server->audioPolicyService_.audioEcManager_.UpdateStreamCommonInfo(moduleInfo, targetInfo, sourceType);

    server->audioPolicyService_.audioEcManager_.isEcFeatureEnable_ = false;
    server->audioPolicyService_.audioEcManager_.UpdateStreamCommonInfo(moduleInfo, targetInfo, sourceType);
    EXPECT_EQ(server->audioPolicyService_.audioEcManager_.isEcFeatureEnable_, false);
}

/**
 * @tc.name  : Test UpdateStreamMicRefInfo.
 * @tc.number: UpdateStreamMicRefInfo_001
 * @tc.desc  : Test UpdateStreamMicRefInfo interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, UpdateStreamMicRefInfo_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioModuleInfo moduleInfo;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_MESSAGE;

    server->audioPolicyService_.audioEcManager_.UpdateStreamMicRefInfo(moduleInfo, sourceType);
    EXPECT_NE(moduleInfo.micRefChannels, "999");
}

/**
 * @tc.name  : Test TriggerAvailableDeviceChangedCallback.
 * @tc.number: TriggerAvailableDeviceChangedCallback_001
 * @tc.desc  : Test TriggerAvailableDeviceChangedCallback interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, TriggerAvailableDeviceChangedCallback_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    bool isConnected;

    isConnected = true;
    server->audioPolicyService_.audioDeviceStatus_.TriggerAvailableDeviceChangedCallback(desc, isConnected);

    isConnected = false;
    server->audioPolicyService_.audioDeviceStatus_.TriggerAvailableDeviceChangedCallback(desc, isConnected);
    EXPECT_NE(server->audioPolicyService_.audioPolicyServerHandler_, nullptr);
}

/**
 * @tc.name  : Test OffloadStartPlaying.
 * @tc.number: OffloadStartPlaying_001
 * @tc.desc  : Test OffloadStartPlaying interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, OffloadStartPlaying_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::vector<int32_t> sessionIds = {0};
    int32_t ret;

    server->audioPolicyService_.SetA2dpOffloadFlag(BluetoothOffloadState::NO_A2DP_DEVICE);
    ret = server->audioPolicyService_.OffloadStartPlaying(sessionIds);
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyService_.SetA2dpOffloadFlag(BluetoothOffloadState::A2DP_OFFLOAD);
    sessionIds.clear();
    ret = server->audioPolicyService_.OffloadStartPlaying(sessionIds);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test OffloadStopPlaying.
 * @tc.number: OffloadStopPlaying_001
 * @tc.desc  : Test OffloadStopPlaying interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, OffloadStopPlaying_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    std::vector<int32_t> sessionIds = {0};
    int32_t ret;

    server->audioPolicyService_.SetA2dpOffloadFlag(BluetoothOffloadState::NO_A2DP_DEVICE);
    ret = server->audioPolicyService_.OffloadStopPlaying(sessionIds);
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyService_.SetA2dpOffloadFlag(BluetoothOffloadState::A2DP_OFFLOAD);
    sessionIds.clear();
    ret = server->audioPolicyService_.OffloadStopPlaying(sessionIds);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test OffloadGetRenderPosition.
 * @tc.number: OffloadGetRenderPosition_001
 * @tc.desc  : Test OffloadGetRenderPosition interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, OffloadGetRenderPosition_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    uint32_t delayValue = 0;
    uint64_t sendDataSize = 0;
    uint32_t timeStamp = 0;
    int32_t ret;

    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_ = LOCAL_NETWORK_ID;
    server->audioPolicyService_.SetA2dpOffloadFlag(BluetoothOffloadState::NO_A2DP_DEVICE);
    ret = server->audioPolicyService_.OffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_ = REMOTE_NETWORK_ID;
    server->audioPolicyService_.SetA2dpOffloadFlag(BluetoothOffloadState::NO_A2DP_DEVICE);
    ret = server->audioPolicyService_.OffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DeviceType::DEVICE_TYPE_SPEAKER;
    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_ = REMOTE_NETWORK_ID;
    server->audioPolicyService_.SetA2dpOffloadFlag(BluetoothOffloadState::NO_A2DP_DEVICE);
    ret = server->audioPolicyService_.OffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetA2dpOffloadCodecAndSendToDsp.
 * @tc.number: GetA2dpOffloadCodecAndSendToDsp_001
 * @tc.desc  : Test GetA2dpOffloadCodecAndSendToDsp interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetA2dpOffloadCodecAndSendToDsp_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    server->audioPolicyService_.audioA2dpOffloadManager_->GetA2dpOffloadCodecAndSendToDsp();

    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    server->audioPolicyService_.audioA2dpOffloadManager_->GetA2dpOffloadCodecAndSendToDsp();
    EXPECT_EQ(server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_,
        DeviceType::DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name  : Test OnPreferredStateUpdated.
 * @tc.number: OnPreferredStateUpdated_001
 * @tc.desc  : Test OnPreferredStateUpdated interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, OnPreferredStateUpdated_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioDeviceDescriptor desc;
    DeviceInfoUpdateCommand updateCommand;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;

    updateCommand = DeviceInfoUpdateCommand::CATEGORY_UPDATE;
    desc.deviceCategory_ = DeviceCategory::BT_UNWEAR_HEADPHONE;
    desc.deviceType_ = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.macAddress_ = server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.macAddress_;
    server->audioPolicyService_.audioDeviceStatus_.OnPreferredStateUpdated(desc, updateCommand, reason);

    desc.deviceType_ = DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    desc.macAddress_ = server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.macAddress_;
    server->audioPolicyService_.audioDeviceStatus_.OnPreferredStateUpdated(desc, updateCommand, reason);

    desc.deviceCategory_ = DeviceCategory::BT_HEADPHONE;
    desc.deviceType_ = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    server->audioPolicyService_.audioDeviceStatus_.OnPreferredStateUpdated(desc, updateCommand, reason);

    desc.deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    server->audioPolicyService_.audioDeviceStatus_.OnPreferredStateUpdated(desc, updateCommand, reason);

    updateCommand = DeviceInfoUpdateCommand::ENABLE_UPDATE;
    server->audioPolicyService_.audioDeviceStatus_.OnPreferredStateUpdated(desc, updateCommand, reason);

    EXPECT_EQ(reason.isOverride(), false);
}

/**
 * @tc.name  : Test CheckAndActiveHfpDevice.
 * @tc.number: CheckAndActiveHfpDevice_001
 * @tc.desc  : Test CheckAndActiveHfpDevice interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, CheckAndActiveHfpDevice_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioDeviceDescriptor desc;

    server->audioPolicyService_.audioDeviceStatus_.CheckAndActiveHfpDevice(desc);

    desc.connectState_ = ConnectState::DEACTIVE_CONNECTED;
    desc.deviceType_ = DeviceType::DEVICE_TYPE_MIC;
    server->audioPolicyService_.audioDeviceStatus_.CheckAndActiveHfpDevice(desc);

    desc.connectState_ = ConnectState::CONNECTED;
    desc.deviceType_ = DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    server->audioPolicyService_.audioDeviceStatus_.CheckAndActiveHfpDevice(desc);
    EXPECT_EQ(desc.deviceType_, DeviceType::DEVICE_TYPE_BLUETOOTH_SCO);
}

/**
 * @tc.name  : Test SetDeviceActive.
 * @tc.number: SetDeviceActive_001
 * @tc.desc  : Test SetDeviceActive interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, SetDeviceActive_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    auto ret = server->audioPolicyService_.audioDeviceLock_.SetDeviceActive(DeviceType::DEVICE_TYPE_SPEAKER, true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetAvailableDevices.
 * @tc.number: GetAvailableDevices_001
 * @tc.desc  : Test GetAvailableDevices interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, GetAvailableDevices_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioStreamChangeInfo streamChangeInfo;
    server->audioPolicyService_.audioDeviceLock_.FetchOutputDeviceForTrack(
        streamChangeInfo, AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    server->audioPolicyService_.audioDeviceLock_.FetchInputDeviceForTrack(streamChangeInfo);
    auto ret = server->audioPolicyService_.audioDeviceLock_.GetAvailableDevices(AudioDeviceUsage::ALL_MEDIA_DEVICES);
    EXPECT_GT(ret.size(), 0);
}

/**
 * @tc.name  : Test RegisterTracker.
 * @tc.number: RegisterTracker_001
 * @tc.desc  : Test RegisterTracker interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, RegisterTracker_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1001;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 2001;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RendererState::RENDERER_RUNNING;

    sptr<AudioClientTrackerCallbackStub> callback = new AudioClientTrackerCallbackStub();
    std::shared_ptr<AudioClientTracker> clientTrackerObj = nullptr;
    callback->SetClientTrackerCallback(clientTrackerObj);
    sptr<IRemoteObject> object = callback->AsObject();
    auto ret = server->audioPolicyService_.audioDeviceLock_.RegisterTracker(
        mode, streamChangeInfo, object, API_VERSION::API_9);
    EXPECT_EQ(ret, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
