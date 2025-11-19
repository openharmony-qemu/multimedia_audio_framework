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
static const std::string PIPE_ACCESSORY_INPUT_UNITTEST = "accessory_input";
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
 * @tc.name  : Test WriteAllDeviceSysEvents.
 * @tc.number: WriteAllDeviceSysEvents_001
 * @tc.desc  : Test WriteAllDeviceSysEvents interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, WriteAllDeviceSysEvents_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_TRUE(server != nullptr);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    bool isConnected;

    isConnected = false;
    desc = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::ALL_DEVICES_FLAG);
    server->audioPolicyService_.audioDeviceStatus_.WriteAllDeviceSysEvents(desc, isConnected);
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
    server->audioPolicyService_.audioVolumeManager_.SetAbsVolumeSceneAsync(macAddress, support, 0);
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
    int32_t ret = server->audioVolumeManager_.SetDeviceAbsVolumeSupported(macAddress, support, 0);
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
    EXPECT_EQ(volumeLevelRet, 8);

    isA2dpDevice = false;
    volumeLevelRet = server->audioPolicyService_.audioVolumeManager_.DealWithSafeVolume(volumeLevel, isA2dpDevice);
    EXPECT_EQ(volumeLevelRet, 8);

    volumeLevel = 11;
    volumeLevelRet = server->audioPolicyService_.audioVolumeManager_.DealWithSafeVolume(volumeLevel, isA2dpDevice);
    EXPECT_EQ(volumeLevelRet, 8);
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
    EXPECT_EQ(safeVolumeLevel, 8);

    volumeLevel = 8;
    safeVolumeLevel
        = server->audioPolicyService_.audioVolumeManager_.HandleAbsBluetoothVolume(macAddress, volumeLevel);
    EXPECT_EQ(safeVolumeLevel, 8);
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
    int32_t ret = server->audioVolumeManager_.SetA2dpDeviceVolume(macAddress, volumeLevel, internalCall);
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
    int32_t ret = server->audioVolumeManager_.SetA2dpDeviceVolume(macAddress, volumeLevel, internalCall);
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
    EXPECT_EQ(ret, AUDIO_FLAG_VOIP_FAST);

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
    EXPECT_EQ(targetSource, SourceType::SOURCE_TYPE_CAMCORDER);
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

    deviceType = DeviceType::DEVICE_TYPE_ACCESSORY;
    pipeNameByDeviceForEc = server->audioPolicyService_.audioEcManager_.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(pipeNameByDeviceForEc, PIPE_ACCESSORY_INPUT_UNITTEST);

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
    std::shared_ptr<AdapterPipeInfo> pipeInfo;

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
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
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
    EXPECT_EQ(ecSamplingRate, "48000");
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
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
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
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();

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
    ASSERT_TRUE(server != nullptr);
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
}

/**
 * @tc.name  : Test UpdateStreamCommonInfo.
 * @tc.number: UpdateStreamCommonInfo_001
 * @tc.desc  : Test UpdateStreamCommonInfo interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, UpdateStreamCommonInfo_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_TRUE(server != nullptr);
    AudioModuleInfo moduleInfo;
    PipeStreamPropInfo targetInfo;
    SourceType sourceType = SourceType::SOURCE_TYPE_MIC;

    server->audioPolicyService_.audioEcManager_.isEcFeatureEnable_ = true;
    server->audioPolicyService_.audioEcManager_.UpdateStreamCommonInfo(moduleInfo, targetInfo, sourceType);

    server->audioPolicyService_.audioEcManager_.isEcFeatureEnable_ = false;
    server->audioPolicyService_.audioEcManager_.UpdateStreamCommonInfo(moduleInfo, targetInfo, sourceType);
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
    ret = server->audioPolicyService_.OffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_ = REMOTE_NETWORK_ID;
    ret = server->audioPolicyService_.OffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, SUCCESS);

    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DeviceType::DEVICE_TYPE_SPEAKER;
    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_ = REMOTE_NETWORK_ID;
    ret = server->audioPolicyService_.OffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NearlinkGetRenderPosition.
 * @tc.number: NearlinkGetRenderPosition_001
 * @tc.desc  : Test NearlinkGetRenderPosition interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, NearlinkGetRenderPosition_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_TRUE(server != nullptr);
    uint32_t delayValue = 0;
    int32_t ret;

    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DeviceType::DEVICE_TYPE_SPEAKER;
    ret = server->audioPolicyService_.NearlinkGetRenderPosition(delayValue);
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
    ASSERT_TRUE(server != nullptr);
    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    server->audioPolicyService_.audioA2dpOffloadManager_->GetA2dpOffloadCodecAndSendToDsp();

    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    server->audioPolicyService_.audioA2dpOffloadManager_->GetA2dpOffloadCodecAndSendToDsp();
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

    EXPECT_EQ(reason.IsOverride(), false);
}

/**
 * @tc.name  : Test CheckAndActiveHfpDevice.
 * @tc.number: CheckAndActiveHfpDevice_001
 * @tc.desc  : Test CheckAndActiveHfpDevice interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, CheckAndActiveHfpDevice_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_TRUE(server != nullptr);
    AudioDeviceDescriptor desc;

    server->audioPolicyService_.audioDeviceStatus_.CheckAndActiveHfpDevice(desc);

    desc.connectState_ = ConnectState::DEACTIVE_CONNECTED;
    desc.deviceType_ = DeviceType::DEVICE_TYPE_MIC;
    server->audioPolicyService_.audioDeviceStatus_.CheckAndActiveHfpDevice(desc);

    desc.connectState_ = ConnectState::CONNECTED;
    desc.deviceType_ = DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    server->audioPolicyService_.audioDeviceStatus_.CheckAndActiveHfpDevice(desc);
}

/**
 * @tc.name  : Test IsDevicePlaybackSupported.
 * @tc.number: IsDevicePlaybackSupported_001
 * @tc.desc  : Test IsDevicePlaybackSupported interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, IsDevicePlaybackSupported_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioProcessConfig config;
    config.streamInfo.encoding = ENCODING_EAC3;
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    auto ret = server->audioPolicyService_.IsDevicePlaybackSupported(config, desc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsDevicePlaybackSupported.
 * @tc.number: IsDevicePlaybackSupported_002
 * @tc.desc  : Test IsDevicePlaybackSupported interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, IsDevicePlaybackSupported_002, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioProcessConfig config;
    config.streamInfo.encoding = ENCODING_PCM;
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    auto ret = server->audioPolicyService_.IsDevicePlaybackSupported(config, desc);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsDevicePlaybackSupported.
 * @tc.number: IsDevicePlaybackSupported_003
 * @tc.desc  : Test IsDevicePlaybackSupported interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, IsDevicePlaybackSupported_003, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioProcessConfig config;
    config.streamInfo.encoding = ENCODING_EAC3;
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DeviceType::DEVICE_TYPE_HDMI;
    auto ret = server->audioPolicyService_.IsDevicePlaybackSupported(config, desc);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsDevicePlaybackSupported.
 * @tc.number: IsDevicePlaybackSupported_004
 * @tc.desc  : Test IsDevicePlaybackSupported interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, IsDevicePlaybackSupported_004, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    AudioProcessConfig config;
    config.streamInfo.encoding = ENCODING_EAC3;
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DeviceType::DEVICE_TYPE_LINE_DIGITAL;
    auto ret = server->audioPolicyService_.IsDevicePlaybackSupported(config, desc);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test RegisterAccessibilityMonitorHelper.
* @tc.number: RegisterAccessibilityMonitorHelperTest
* @tc.desc  : Test RegisterAccessibilityMonitorHelper interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, RegisterAccessibilityMonitorHelperTest, TestSize.Level1)
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    bool isDataShareReady = settingProvider.isDataShareReady_.load();
    settingProvider.SetDataShareReady(true);
    ASSERT_TRUE(settingProvider.isDataShareReady_.load());
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_TRUE(server != nullptr);
    // The result can be verified only after the datashare mock framework is completed.
    server->audioPolicyService_.RegisterAccessibilityMonitorHelper();
    settingProvider.SetDataShareReady(isDataShareReady);
}

/**
* @tc.name  : Test RegisterDataObserver.
* @tc.number: RegisterDataObserverTest
* @tc.desc  : Test RegisterDataObserver interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, RegisterDataObserverTest, TestSize.Level1)
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    bool isDataShareReady = settingProvider.isDataShareReady_.load();
    settingProvider.SetDataShareReady(true);
    ASSERT_TRUE(settingProvider.isDataShareReady_.load());
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_TRUE(server != nullptr);
    // These result can be verified only after the datashare mock framework is completed.
    server->audioPolicyService_.audioConnectedDevice_.RegisterNameMonitorHelper();
    server->audioPolicyService_.audioPolicyManager_.RegisterDoNotDisturbStatus();
    server->audioPolicyService_.audioPolicyManager_.RegisterDoNotDisturbStatusWhiteList();
    settingProvider.SetDataShareReady(isDataShareReady);
}

/**
 * @tc.name  : Test AudioPolicyServiceExtUnitTest.
 * @tc.number: OnForcedDeviceSelected_01
 * @tc.desc  : Test OnForcedDeviceSelected interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, OnForcedDeviceSelected_01, TestSize.Level1)
{
    AudioDeviceDescriptor devDesc;
    devDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    devDesc.macAddress_ = "11:22";
    devDesc.deviceRole_ = OUTPUT_DEVICE;
    devDesc.deviceId_ = 1234;
    AudioDeviceManager::GetAudioDeviceManager().AddNewDevice(make_shared<AudioDeviceDescriptor>(devDesc));
    auto &service = AudioPolicyService::GetAudioPolicyService();
    service.OnForcedDeviceSelected(devDesc.deviceType_, devDesc.macAddress_);
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test AudioPolicyServiceExtUnitTest.
 * @tc.number: OnPrivacyDeviceSelected_01
 * @tc.desc  : Test OnPrivacyDeviceSelected interfaces.
 */
HWTEST_F(AudioPolicyServiceExtUnitTest, OnPrivacyDeviceSelected_01, TestSize.Level1)
{
    AudioDeviceDescriptor devDesc;
    devDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    devDesc.macAddress_ = "11:22";
    devDesc.deviceRole_ = OUTPUT_DEVICE;
    devDesc.deviceId_ = 5678;
    AudioDeviceManager::GetAudioDeviceManager().AddNewDevice(make_shared<AudioDeviceDescriptor>(devDesc));
    auto &service = AudioPolicyService::GetAudioPolicyService();
    service.OnPrivacyDeviceSelected(devDesc.deviceType_, devDesc.macAddress_);
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(server, nullptr);
}

/**
* @tc.name  : Test GetsinkPortName.
* @tc.number: GetSinkPortName_004
* @tc.desc  : Test RegisterDataObserver interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, GetSinkPortName_004, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_NONE;
    std::string retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType);
    EXPECT_EQ(PORT_NONE, retPortName);

    deviceType = DEVICE_TYPE_HEARING_AID;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType);
    EXPECT_EQ(HEARING_AID_SPEAKER, retPortName);
}

/**
* @tc.name  : Test SetSystemVolumeDegree
* @tc.number: SetSystemVolumeDegree_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, SetSystemVolumeDegree_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_TRUE(server != nullptr);
    int32_t streamType = static_cast<int32_t>(STREAM_MUSIC);
    int32_t volumeDegree = 44;
    int32_t ret = server->SetSystemVolumeDegree(streamType, volumeDegree, 0, 0);
    EXPECT_EQ(ret, SUCCESS);

    int32_t streamType2 = static_cast<int32_t>(STREAM_APP);
    ret = server->SetSystemVolumeDegree(streamType2, volumeDegree, 0, 0);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    server->GetSystemVolumeDegree(STREAM_ALL, 0, ret);
    EXPECT_EQ(ret, volumeDegree);

    server->GetMinVolumeDegree(streamType, DEVICE_TYPE_NONE, ret);
    EXPECT_EQ(ret, 0);

    auto &manager = static_cast<AudioAdapterManager &>(server->audioPolicyManager_);
    manager.isVolumeUnadjustable_ = true;
    ret = server->SetSystemVolumeDegree(streamType, volumeDegree, 0, 0);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    manager.isVolumeUnadjustable_ = false;

    ret = server->SetSystemVolumeDegree(static_cast<int32_t>(STREAM_ALL), volumeDegree, 0, 0);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test SetSystemVolumeDegree
* @tc.number: SetSystemVolumeDegree_002
* @tc.desc  : Test AudioPolicyService differernt degree.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, SetSystemVolumeDegree_002, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_TRUE(server != nullptr);
    int32_t streamType = static_cast<int32_t>(STREAM_MUSIC);
    int32_t volumeDegree1 = -1;
    int32_t volumeDegree2 = 0;
    int32_t volumeDegree3 = 1;
    int32_t volumeDegree4 = 99;
    int32_t volumeDegree5 = 100;
    int32_t volumeDegree6 = 101;

    int32_t volumelevel1 = 1;
    int32_t volumelevel2 = 14;

    int32_t ret = server->SetSystemVolumeDegree(streamType, volumeDegree1, 0, 0);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = server->SetSystemVolumeDegree(streamType, volumeDegree6, 0, 0);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = server->SetSystemVolumeDegree(streamType, volumeDegree2, 0, 0);
    EXPECT_EQ(ret, SUCCESS);

    ret = server->SetSystemVolumeDegree(streamType, volumeDegree5, 0, 0);
    EXPECT_EQ(ret, SUCCESS);

    ret = server->SetSystemVolumeDegree(streamType, volumeDegree3, 0, 0);
    EXPECT_EQ(ret, SUCCESS);

    server->GetSystemVolumeLevel(streamType, 0, ret);
    EXPECT_EQ(ret, volumelevel1);

    ret = server->SetSystemVolumeDegree(streamType, volumeDegree4, 0, 0);
    EXPECT_EQ(ret, SUCCESS);

    server->GetSystemVolumeLevel(streamType, 0, ret);
    EXPECT_EQ(ret, volumelevel2);
}

/**
* @tc.name  : Test SetSystemVolumeDegree
* @tc.number: SetSystemVolumeDegree_003
* @tc.desc  : Test AudioPolicyService differernt degree by set level.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, SetSystemVolumeDegree_003, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_TRUE(server != nullptr);
    int32_t streamType = static_cast<int32_t>(STREAM_MUSIC);
    int32_t volumeLevel1 = 1;
    int32_t volumeLevel2 = 15;

    int32_t volumeDegree2 = 100;
    int32_t ret = server->SetSystemVolumeLevel(streamType, volumeLevel1, 0, 0);
    EXPECT_EQ(ret, SUCCESS);

    server->GetSystemVolumeDegree(streamType, 0, ret);
    EXPECT_NE(ret, 0);

    ret = server->SetSystemVolumeLevel(streamType, volumeLevel2, 0, 0);
    EXPECT_EQ(ret, SUCCESS);

    server->GetSystemVolumeDegree(streamType, 0, ret);
    EXPECT_EQ(ret, volumeDegree2);
}

/**
* @tc.name  : Test SetSystemVolumeDegree
* @tc.number: SetSystemVolumeDegree_004
* @tc.desc  : Test AudioPolicyService differernt degree by set level.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, SetSystemVolumeDegree_004, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    ASSERT_TRUE(server != nullptr);
    int32_t streamType1 = static_cast<int32_t>(STREAM_MUSIC);
    int32_t volumeDegree1 = 0;
    int32_t volumeDegree2 = 100;

    int32_t success = 0;
    for (int32_t i = volumeDegree1; i <= volumeDegree2; ++i) {
        AUDIO_INFO_LOG("level1=%{public}d", i);
        int32_t ret = server->SetSystemVolumeDegree(streamType1, i, 0, 0);
        if (ret == SUCCESS) {
            success++;
        } else {
            AUDIO_INFO_LOG("level1=%{public}d, failed", i);
        }
    }

    EXPECT_EQ(volumeDegree2 - volumeDegree1 + 1, success);
}

} // namespace AudioStandard
} // namespace OHOS
