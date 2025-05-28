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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_policy_server.h"
#include "message_parcel.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
bool g_hasPermission = false;
constexpr int32_t OFFSET = 4;
const int32_t MOD_NUM_TWO = 2;
const int32_t CONNECTING_NUMBER = 10;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
const int32_t LIMITSIZE = 4;
const int32_t SHIFT_LEFT_8 = 8;
const int32_t SHIFT_LEFT_16 = 16;
const int32_t SHIFT_LEFT_24 = 24;
const uint32_t LIMIT_ONE = 0;
const uint32_t LIMIT_TWO = 30;
const uint32_t LIMIT_THREE = 60;
const uint32_t LIMIT_FOUR = static_cast<uint32_t>(AudioPolicyInterfaceCode::AUDIO_POLICY_MANAGER_CODE_MAX);
bool g_hasServerInit = false;
const uint8_t TESTSIZE = 6;
typedef void (*TestPtr)(const uint8_t *, size_t);

AudioPolicyServer* GetServerPtr()
{
    static AudioPolicyServer server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit) {
        server.OnStart();
        server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server.audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return &server;
}

uint32_t Convert2Uint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    /* Move the 0th digit to the left by 24 bits, the 1st digit to the left by 16 bits,
       the 2nd digit to the left by 8 bits, and the 3rd digit not to the left */
    return (ptr[0] << SHIFT_LEFT_24) | (ptr[1] << SHIFT_LEFT_16) | (ptr[2] << SHIFT_LEFT_8) | (ptr[3]);
}

void AudioFuzzTestGetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
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
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audiofuzztest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

void AudioPolicyServiceDumpTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t code = Convert2Uint32(rawData) % (LIMIT_TWO - LIMIT_ONE + 1) + LIMIT_ONE;

    rawData = rawData + OFFSET;
    size = size - OFFSET;

    std::string dumpStr = "";
    GetServerPtr()->audioPolicyDump_.DevicesInfoDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.AudioModeDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.AudioPolicyParserDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.XmlParsedDataMapDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.StreamVolumesDump(dumpStr);
    std::map<DeviceVolumeType, std::shared_ptr<DeviceVolumeInfo>> deviceVolumeInfoMap;
    GetServerPtr()->audioPolicyDump_.DeviceVolumeInfosDump(dumpStr, deviceVolumeInfoMap);
    GetServerPtr()->audioPolicyDump_.AudioStreamDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.GetVolumeConfigDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.GetGroupInfoDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.GetCallStatusDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.GetRingerModeDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.GetMicrophoneDescriptorsDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.GetCapturerStreamDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.GetOffloadStatusDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.EffectManagerInfoDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.MicrophoneMuteInfoDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.GetVolumeConfigDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.GetVolumeConfigDump(dumpStr);
    GetServerPtr()->audioPolicyDump_.GetVolumeConfigDump(dumpStr);

    GetServerPtr()->interruptService_->AudioInterruptZoneDump(dumpStr);
}

void AudioPolicyServiceDeviceTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint8_t num = *reinterpret_cast<const uint8_t *>(rawData);
    DeviceType deviceType = static_cast<DeviceType>(num % DEVICE_TYPE_FILE_SOURCE);
    bool isConnected = static_cast<bool>(num % MOD_NUM_TWO);
    AudioDeviceDescriptor audioDeviceDescriptor;
    GetServerPtr()->audioPolicyService_.OnPnpDeviceStatusUpdated(audioDeviceDescriptor, isConnected);

    GetServerPtr()->audioPolicyService_.IsA2dpOffloadConnected();
    int32_t state = (num % MOD_NUM_TWO) + CONNECTING_NUMBER; // DATA_LINK_CONNECTING = 10, DATA_LINK_CONNECTED = 11;
    GetServerPtr()->audioPolicyService_.audioDeviceLock_.UpdateSessionConnectionState(num, state);

    std::string macAddress = "11:22:33:44:55:66";
    GetServerPtr()->audioPolicyService_.SetCallDeviceActive(deviceType, isConnected, macAddress);

    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->UpdateOffloadWhenActiveDeviceSwitchFromA2dp();
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->GetA2dpOffloadCodecAndSendToDsp();
    GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.UpdateAudioCapturerMicrophoneDescriptor(deviceType);

    // the max value of BluetoothOffloadState is A2DP_OFFLOAD.
    BluetoothOffloadState flag = static_cast<BluetoothOffloadState>(num % (A2DP_OFFLOAD + 1));
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleA2dpDeviceInOffload(flag);
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleA2dpDeviceOutOffload(flag);

    uint32_t sessionId = static_cast<uint32_t>(num);
    // the max value of AudioPipeType is PIPE_TYPE_DIRECT_VOIP.
    AudioPipeType pipeType = static_cast<AudioPipeType>(num % (PIPE_TYPE_DIRECT_VOIP + 1));
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.MoveToNewPipeInner(sessionId, pipeType);

    GetServerPtr()->audioPolicyService_.audioOffloadStream_.LoadMchModule();
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.ConstructMchAudioModuleInfo(deviceType);
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.LoadOffloadModule();
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.UnloadOffloadModule();
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.ConstructOffloadAudioModuleInfo(deviceType);
}

void AudioPolicyServiceAccountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    uint8_t num = *reinterpret_cast<const uint8_t *>(rawData);
    GetServerPtr()->audioPolicyService_.GetCurActivateCount();
    GetServerPtr()->audioPolicyService_.NotifyAccountsChanged(num);
}

void AudioPolicyServiceSafeVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t code = Convert2Uint32(rawData) % (LIMIT_TWO - LIMIT_ONE + 1) + LIMIT_ONE;

    rawData = rawData + OFFSET;
    size = size - OFFSET;

    GetServerPtr()->audioPolicyService_.audioVolumeManager_.SetDeviceSafeVolumeStatus();
    GetServerPtr()->audioPolicyService_.DisableSafeMediaVolume();
}

void AudioPolicyServiceInterfaceTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    bool fuzzBool = *reinterpret_cast<const bool *>(rawData);
    int32_t fuzzInt32One = *reinterpret_cast<const int8_t *>(rawData);
    int32_t fuzzInt32Two = *reinterpret_cast<const int8_t *>(rawData);
    int32_t fuzzInt32Three = *reinterpret_cast<const int8_t *>(rawData);
    int32_t fuzzFloat = *reinterpret_cast<const float *>(rawData);
    std::string fuzzNetworkId = "FUZZNETWORKID";
    std::string fuzzString(reinterpret_cast<const char*>(rawData), size - 1);

    AudioStreamType fuzzAudioStreamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    DeviceType fuzzDeviceType = *reinterpret_cast<const DeviceType *>(rawData);
    DeviceRole fuzzDeviceRole = *reinterpret_cast<const DeviceRole *>(rawData);
    StreamUsage fuzzStreamUsage = *reinterpret_cast<const StreamUsage *>(rawData);

    AudioDeviceDescriptor fuzzAudioDeviceDescriptor;
    std::shared_ptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> fuzzAudioDeviceDescriptorSptrVector;
    fuzzAudioDeviceDescriptorSptrVector.push_back(fuzzAudioDeviceDescriptorSptr);
    sptr<AudioRendererFilter> fuzzAudioRendererFilter = new AudioRendererFilter();

    // set offload support on for covery
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.isOffloadAvailable_ = true;
    GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.HandleRecoveryPreferredDevices(fuzzInt32One,
        fuzzInt32Two, fuzzInt32Three);
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->GetVolumeGroupType(fuzzDeviceType);
    GetServerPtr()->audioPolicyService_.GetSystemVolumeDb(fuzzAudioStreamType);
    GetServerPtr()->audioPolicyService_.SetLowPowerVolume(fuzzInt32One, fuzzFloat);
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.SetOffloadMode();
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.ResetOffloadMode(fuzzInt32One);
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.OffloadStreamReleaseCheck(fuzzInt32One);
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.RemoteOffloadStreamRelease(fuzzInt32One);
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.CheckActiveOutputDeviceSupportOffload();
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.GetOffloadAvailableFromXml();
    GetServerPtr()->audioPolicyService_.SetSourceOutputStreamMute(fuzzInt32One, fuzzBool);
    GetServerPtr()->audioPolicyService_.NotifyRemoteRenderState(fuzzNetworkId, fuzzString, fuzzString);
    GetServerPtr()->audioPolicyService_.IsArmUsbDevice(fuzzAudioDeviceDescriptor);
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.IsDeviceConnected(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.DeviceParamsCheck(fuzzDeviceRole,
        fuzzAudioDeviceDescriptorSptrVector);
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.NotifyUserSelectionEventToBt(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SetRenderDeviceForUsage(fuzzStreamUsage,
        fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectOutputDevice(
        fuzzAudioRendererFilter, fuzzAudioDeviceDescriptorSptrVector);
    GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.WriteSelectOutputSysEvents(
        fuzzAudioDeviceDescriptorSptrVector, fuzzStreamUsage);
    GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectFastOutputDevice(
        fuzzAudioRendererFilter, fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.FilterSourceOutputs(fuzzInt32One);
    GetServerPtr()->audioPolicyService_.OnPnpDeviceStatusUpdated(fuzzAudioDeviceDescriptor, fuzzBool);
}

void AudioDeviceConnectTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    // Coverage first
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamInfo.channels = AudioChannel::STEREO;
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_WIRED_HEADSET, true,
        "", "fuzzDevice", streamInfo);
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_WIRED_HEADSET, false,
        "", "fuzzDevice", streamInfo);

    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_USB_HEADSET, true,
        "", "fuzzDevice", streamInfo);
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_USB_HEADSET, false,
        "", "fuzzDevice", streamInfo);

    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_DP, true,
        "", "fuzzDevice", streamInfo);
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_DP, false,
        "", "fuzzDevice", streamInfo);

    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, true,
        "08:00:20:0A:8C:6D", "fuzzBtDevice", streamInfo);
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, false,
        "08:00:20:0A:8C:6D", "fuzzBtDevice", streamInfo);

    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, true,
        "08:00:20:0A:8C:6D", "fuzzBtDevice", streamInfo);
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, false,
        "08:00:20:0A:8C:6D", "fuzzBtDevice", streamInfo);

    bool fuzzBool = *reinterpret_cast<const bool *>(rawData);
    std::string fuzzString(reinterpret_cast<const char*>(rawData), size - 1);
    DeviceType fuzzDeviceType = *reinterpret_cast<const DeviceType *>(rawData);
    AudioSamplingRate fuzzAudioSamplingRate = *reinterpret_cast<const AudioSamplingRate *>(rawData);
    AudioChannel fuzzAudioChannel = *reinterpret_cast<const AudioChannel *>(rawData);
    AudioSampleFormat fuzzAudioSampleFormat = *reinterpret_cast<const AudioSampleFormat *>(rawData);
    streamInfo.samplingRate = fuzzAudioSamplingRate;
    streamInfo.channels = fuzzAudioChannel;
    streamInfo.format = fuzzAudioSampleFormat;

    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(fuzzDeviceType, fuzzBool, fuzzString,
        "fuzzDevice", streamInfo);
}
} // namespace AudioStandard
} // namesapce OHOS

OHOS::AudioStandard::TestPtr g_testPtrs[OHOS::AudioStandard::TESTSIZE] = {
    OHOS::AudioStandard::AudioPolicyServiceDumpTest,
    OHOS::AudioStandard::AudioPolicyServiceDeviceTest,
    OHOS::AudioStandard::AudioPolicyServiceAccountTest,
    OHOS::AudioStandard::AudioPolicyServiceSafeVolumeTest,
    OHOS::AudioStandard::AudioPolicyServiceInterfaceTest,
    OHOS::AudioStandard::AudioDeviceConnectTest
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= 1) {
        return 0;
    }
    uint8_t firstByte = *data % OHOS::AudioStandard::TESTSIZE;
    if (firstByte >= OHOS::AudioStandard::TESTSIZE) {
        return 0;
    }
    data = data + 1;
    size = size - 1;
    g_testPtrs[firstByte](data, size);
    return 0;
}