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
#include "audio_policy_service.h"
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
const uint8_t TESTSIZE = 73;
typedef void (*TestPtr)(const uint8_t *, size_t);

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
static int32_t NUM_2 = 2;

typedef void (*TestFuncs)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

class RemoteObjectTestStub : public IRemoteObject {
public:
    RemoteObjectTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectTestStub");
};

sptr<AudioPolicyServer> GetServerPtr()
{
    static sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit && server != nullptr) {
        server->OnStart();
        server->OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server->OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server->OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server->OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server->OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server->audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return server;
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

    int32_t state = (num % MOD_NUM_TWO) + CONNECTING_NUMBER; // DATA_LINK_CONNECTING = 10, DATA_LINK_CONNECTED = 11;

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
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.SetOffloadMode();
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.ResetOffloadMode(fuzzInt32One);
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.OffloadStreamReleaseCheck(fuzzInt32One);
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.RemoteOffloadStreamRelease(fuzzInt32One);
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.CheckActiveOutputDeviceSupportOffload();
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.GetOffloadAvailableFromXml();
    GetServerPtr()->audioPolicyService_.SetSourceOutputStreamMute(fuzzInt32One, fuzzBool);
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

void AudioPolicyServiceSubscribeSafeVolumeEventFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.SubscribeSafeVolumeEvent();
}

void AudioPolicyServiceOnReceiveEventFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    EventFwk::CommonEventData eventData;
    OHOS::EventFwk::Want want;
    want.SetAction("usual.event.LOCALE_CHANGED");
    eventData.SetWant(want);
    GetServerPtr()->audioPolicyService_.OnReceiveEvent(eventData);
}

void AudioPolicyServiceSetAppVolumeLevelFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    int32_t appUid = GetData<int32_t>() % NUM_2;
    int32_t volumeLevel = GetData<int32_t>() % NUM_2;
    GetServerPtr()->audioPolicyService_.SetAppVolumeLevel(appUid, volumeLevel);
}

void AudioPolicyServiceSetSourceOutputStreamMuteFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    int32_t uid = GetData<int32_t>() % NUM_2;
    bool setMute = true;
    GetServerPtr()->audioPolicyService_.SetSourceOutputStreamMute(uid, setMute);
}

void AudioPolicyServiceGetSelectedDeviceInfoFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    int32_t uid = GetData<int32_t>() % NUM_2;
    int32_t pid = GetData<int32_t>() % NUM_2;
    AudioStreamType fuzzAudioStreamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(uid, pid, fuzzAudioStreamType);
}

void AudioPolicyServiceGetDistributedRoutingRoleInfoFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.GetDistributedRoutingRoleInfo();
}

void AudioPolicyServiceNotifyCapturerAddedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    uint32_t sessionId = GetData<uint32_t>() % NUM_2;
    AudioStreamInfo streamInfo;
    AudioCapturerInfo capturerInfo;
    GetServerPtr()->audioPolicyService_.NotifyCapturerAdded(capturerInfo, streamInfo, sessionId);
}

void AudioPolicyServiceNotifyWakeUpCapturerRemovedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.NotifyWakeUpCapturerRemoved();
}

void AudioPolicyServiceGetFastStreamInfoFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.GetFastStreamInfo();
}

void AudioPolicyServiceIsAbsVolumeSupportedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.IsAbsVolumeSupported();
}


void AudioPolicyServiceGetDevicesFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    DeviceFlag deviceFlag = DeviceFlag::ALL_DEVICES_FLAG;
    GetServerPtr()->audioPolicyService_.GetDevices(deviceFlag);
}

void AudioPolicyServiceGetPreferredInputDeviceDescriptorsFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioCapturerInfo captureInfo;
    std::string networkId = "";
    GetServerPtr()->audioPolicyService_.GetPreferredInputDeviceDescriptors(captureInfo, networkId);
}

void AudioPolicyServiceGetPreferredOutputDeviceDescInnerFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioRendererInfo rendererInfo;
    std::string networkId = "";
    GetServerPtr()->audioPolicyService_.GetPreferredOutputDeviceDescInner(rendererInfo, networkId);
}

void AudioPolicyServiceGetOutputDeviceFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    sptr<AudioRendererFilter> fuzzAudioRendererFilter = new AudioRendererFilter();
    GetServerPtr()->audioPolicyService_.GetOutputDevice(fuzzAudioRendererFilter);
}

void AudioPolicyServiceGetInputDeviceFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    sptr<AudioCapturerFilter> audioCapturerFilter(new AudioCapturerFilter());
    GetServerPtr()->audioPolicyService_.GetInputDevice(audioCapturerFilter);
}

void AudioPolicyServiceGetActiveOutputDeviceDescriptorFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    GetServerPtr()->audioPolicyService_.GetActiveOutputDeviceDescriptor();
}

void AudioPolicyServiceOnUpdateAnahsSupportFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::string anahsShowType = "anahsShowType";
    GetServerPtr()->audioPolicyService_.OnUpdateAnahsSupport(anahsShowType);
}

void AudioPolicyServiceOnPnpDeviceStatusUpdatedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    bool isConnected = true;
    AudioDeviceDescriptor audioDeviceDescriptor;
    GetServerPtr()->audioPolicyService_.OnPnpDeviceStatusUpdated(audioDeviceDescriptor, isConnected);
}

void AudioPolicyServiceOnDeviceStatusUpdatedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    bool isConnected = true;
    AudioDeviceDescriptor audioDeviceDescriptor;
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(audioDeviceDescriptor, isConnected);
    DStatusInfo dStatusInfo;
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(dStatusInfo, isConnected);
}

void AudioPolicyServiceUpdateA2dpOffloadFlagBySpatialServiceFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::string macAddress = "11-22-33-44-55-66";
    std::unordered_map<uint32_t, bool> sessionIDToSpatializationEnableMap;
    GetServerPtr()->audioPolicyService_.UpdateA2dpOffloadFlagBySpatialService(macAddress,
        sessionIDToSpatializationEnableMap);
}

void AudioPolicyServiceRegisterRemoteDevStatusCallbackFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    GetServerPtr()->audioPolicyService_.RegisterRemoteDevStatusCallback();
}

void AudioPolicyServiceGetAllSinkInputsFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::vector<SinkInput> sinkInputs;
    GetServerPtr()->audioPolicyService_.GetAllSinkInputs(sinkInputs);
}

void AudioPolicyServiceRegisterAccessibilityMonitorHelperFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.RegisterAccessibilityMonitorHelper();
}

void AudioPolicyServiceOnServiceConnectedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioServiceIndex serviceIndex = AudioServiceIndex::HDI_SERVICE_INDEX;
    GetServerPtr()->audioPolicyService_.OnServiceConnected(serviceIndex);
}

void AudioPolicyServiceOnServiceDisconnectedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioServiceIndex serviceIndex = AudioServiceIndex::HDI_SERVICE_INDEX;
    GetServerPtr()->audioPolicyService_.OnServiceDisconnected(serviceIndex);
}

void AudioPolicyServiceOnForcedDeviceSelectedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    DeviceType devType = DeviceType::DEVICE_TYPE_SPEAKER;
    const std::string macAddress = "11-22-33-44-55-66";
    GetServerPtr()->audioPolicyService_.OnForcedDeviceSelected(devType, macAddress);
}

void AudioPolicyServiceSetAvailableDeviceChangeCallbackFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    int32_t clientId = GetData<uint32_t>() % NUM_2;
    AudioDeviceUsage usage = AudioDeviceUsage::MEDIA_OUTPUT_DEVICES;
    sptr<IRemoteObject> object = nullptr;
    bool hasBTPermission = true;
    GetServerPtr()->audioPolicyService_.SetAvailableDeviceChangeCallback(clientId, usage, object, hasBTPermission);
}

void AudioPolicyServiceSetQueryClientTypeCallbackFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    sptr<IRemoteObject> object = nullptr;
    GetServerPtr()->audioPolicyService_.SetQueryClientTypeCallback(object);
}

void AudioPolicyServiceSetQueryDeviceVolumeBehaviorCallbackFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    sptr<IRemoteObject> object = nullptr;
    GetServerPtr()->audioPolicyService_.SetQueryDeviceVolumeBehaviorCallback(object);
}

void AudioPolicyServiceUpdateCapturerInfoWhenNoPermissionFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    auto audioCapturerChangeInfos = std::shared_ptr<AudioCapturerChangeInfo>();
    bool hasSystemPermission = true;
    AudioCoreService::UpdateCapturerInfoWhenNoPermission(audioCapturerChangeInfos, hasSystemPermission);
}

void AudioPolicyServiceGetCurrentCapturerChangeInfosFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    bool hasSystemPermission = true;
    bool hasBTPermission = true;
    GetServerPtr()->audioPolicyService_.GetCurrentCapturerChangeInfos(audioCapturerChangeInfos,
        hasBTPermission, hasSystemPermission);
}

void AudioPolicyServiceUpdateDescWhenNoBTPermissionFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> fuzzAudioDeviceDescriptorSptrVector;
    fuzzAudioDeviceDescriptorSptrVector.push_back(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.UpdateDescWhenNoBTPermission(fuzzAudioDeviceDescriptorSptrVector);
}

void AudioPolicyServiceGetPreferredOutputStreamTypeFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioRendererInfo rendererInfo;
    std::string bundleName = "bundleName";
    GetServerPtr()->audioPolicyService_.GetPreferredOutputStreamType(rendererInfo, bundleName);
}

void AudioPolicyServiceGetProcessDeviceInfoFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config;
    bool lockFlag = true;
    AudioDeviceDescriptor deviceInfo;
    GetServerPtr()->audioPolicyService_.GetProcessDeviceInfo(config, lockFlag, deviceInfo);
}

void AudioPolicyServiceGetVoipDeviceInfoFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config;
    bool lockFlag = true;
    AudioDeviceDescriptor deviceInfo;
    int32_t type = GetData<uint32_t>() % NUM_2;
    std::shared_ptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList;
    preferredDeviceList.push_back(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.GetVoipDeviceInfo(config, deviceInfo, type, preferredDeviceList);
}

void AudioPolicyServiceInitSharedVolumeFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioSharedMemory> buffer;
    GetServerPtr()->audioPolicyService_.InitSharedVolume(buffer);
}

void AudioPolicyServiceDynamicUnloadModuleFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioPipeType pipeType = PIPE_TYPE_OFFLOAD;
    GetServerPtr()->audioPolicyService_.DynamicUnloadModule(pipeType);
}

void AudioPolicyServiceGetMaxRendererInstancesFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.GetMaxRendererInstances();
}

void AudioPolicyServiceRegisterBluetoothListenerFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.RegisterBluetoothListener();
}

void AudioPolicyServiceUnregisterBluetoothListenerFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.UnregisterBluetoothListener();
}

void AudioPolicyServiceSubscribeAccessibilityConfigObserverFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.SubscribeAccessibilityConfigObserver();
}

void AudioPolicyServiceQueryEffectManagerSceneModeFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    SupportedEffectConfig supportedEffectConfig;
    GetServerPtr()->audioPolicyService_.QueryEffectManagerSceneMode(supportedEffectConfig);
}

void AudioPolicyServiceRegisterDataObserverFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    GetServerPtr()->audioPolicyService_.RegisterDataObserver();
}

void AudioPolicyServiceGetHardwareOutputSamplingRateFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    std::shared_ptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    GetServerPtr()->audioPolicyService_.GetHardwareOutputSamplingRate(fuzzAudioDeviceDescriptorSptr);
}

void AudioPolicyServiceDeviceFilterByUsageInnerFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    AudioDeviceUsage usage = AudioDeviceUsage::MEDIA_OUTPUT_DEVICES;
    std::shared_ptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.DeviceFilterByUsageInner(usage, descs);
}

void AudioPolicyServiceOffloadStartPlayingFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    std::vector<int32_t> sessionIds;
    GetServerPtr()->audioPolicyService_.OffloadStartPlaying(sessionIds);
}

void AudioPolicyServiceOffloadStopPlayingFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    std::vector<int32_t> sessionIds;
    GetServerPtr()->audioPolicyService_.OffloadStopPlaying(sessionIds);
}

void AudioPolicyServiceOffloadGetRenderPositionFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    uint32_t delayValue = GetData<uint32_t>() % NUM_2;
    uint64_t sendDataSize = GetData<uint64_t>() % NUM_2;
    uint32_t timeStamp = GetData<uint32_t>() % NUM_2;
    GetServerPtr()->audioPolicyService_.OffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
}

void AudioPolicyServiceNearlinkGetRenderPositionFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    uint32_t delayValue = GetData<uint32_t>() % NUM_2;
    GetServerPtr()->audioPolicyService_.NearlinkGetRenderPosition(delayValue);
}

void AudioPolicyServiceGetAndSaveClientTypeFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    uint32_t uid = GetData<uint32_t>() % NUM_2;
    std::string bundleName = "bundleName";
    GetServerPtr()->audioPolicyService_.GetAndSaveClientType(uid, bundleName);
}

void AudioPolicyServiceOnDeviceInfoUpdatedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    AudioDeviceDescriptor desc;
    DeviceInfoUpdateCommand command = ENABLE_UPDATE;
    GetServerPtr()->audioPolicyService_.OnDeviceInfoUpdated(desc, command);
}

void AudioPolicyServiceNotifyAccountsChangedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    int id = GetData<int>() % NUM_2;
    GetServerPtr()->audioPolicyService_.NotifyAccountsChanged(id);
}

void AudioPolicyServiceLoadHdiEffectModelFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    GetServerPtr()->audioPolicyService_.LoadHdiEffectModel();
}

void AudioPolicyServiceGetSupportedAudioEffectPropertyFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    AudioEffectPropertyArrayV3 propertyArray;
    GetServerPtr()->audioPolicyService_.GetSupportedAudioEffectProperty(propertyArray);
}

void AudioPolicyServiceGetSupportedEffectPropertyFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    AudioEffectPropertyArrayV3 propertyArray;
    GetServerPtr()->audioPolicyService_.GetSupportedEffectProperty(propertyArray);
}

void AudioPolicyServiceGetSupportedEnhancePropertyFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    AudioEffectPropertyArrayV3 propertyArray;
    GetServerPtr()->audioPolicyService_.GetSupportedEnhanceProperty(propertyArray);
}

void AudioPolicyServiceCheckSupportedAudioEffectPropertyFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    AudioEffectPropertyArrayV3 propertyArray;
    EffectFlag flag = CAPTURE_EFFECT_FLAG;
    GetServerPtr()->audioPolicyService_.CheckSupportedAudioEffectProperty(propertyArray, flag);
}

void AudioPolicyServiceSetAudioEffectPropertyFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    AudioEffectPropertyArrayV3 propertyArray;
    GetServerPtr()->audioPolicyService_.SetAudioEffectProperty(propertyArray);
}

void AudioPolicyServiceGetAudioEnhancePropertyFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    AudioEffectPropertyArrayV3 propertyArrayv3;
    GetServerPtr()->audioPolicyService_.GetAudioEnhanceProperty(propertyArrayv3);
    AudioEnhancePropertyArray propertyArray;
    GetServerPtr()->audioPolicyService_.GetAudioEnhanceProperty(propertyArray);
}

void AudioPolicyServiceGetSupportedAudioEnhancePropertyFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    AudioEnhancePropertyArray propertyArray;
    GetServerPtr()->audioPolicyService_.GetSupportedAudioEnhanceProperty(propertyArray);
}

void AudioPolicyServiceGetAudioEffectPropertyFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    AudioEffectPropertyArrayV3 propertyArrayv3;
    GetServerPtr()->audioPolicyService_.GetAudioEffectProperty(propertyArrayv3);
    AudioEffectPropertyArray propertyArray;
    GetServerPtr()->audioPolicyService_.GetAudioEffectProperty(propertyArray);
}

void AudioPolicyServiceSetAudioEnhancePropertyFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    AudioEnhancePropertyArray propertyArray;
    GetServerPtr()->audioPolicyService_.SetAudioEnhanceProperty(propertyArray);
}

void AudioPolicyServiceGetA2dpOffloadFlagFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    GetServerPtr()->audioPolicyService_.GetA2dpOffloadFlag();
}

void AudioPolicyServiceSetSleAudioOperationCallbackFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    sptr<IRemoteObject> object = nullptr;
    GetServerPtr()->audioPolicyService_.SetSleAudioOperationCallback(object);
}

void AudioPolicyServiceNotifyCapturerRemovedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    uint64_t sessionId = GetData<uint64_t>() % NUM_2;
    GetServerPtr()->audioPolicyService_.NotifyCapturerRemoved(sessionId);
}

void AudioPolicyServiceUpdateSpatializationSupportedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    std::string macAddress = "11-22-33-44-55-66";
    bool support = true;
    GetServerPtr()->audioPolicyService_.UpdateSpatializationSupported(macAddress, support);
}

void AudioPolicyServiceIsDevicePlaybackSupportedFuzztest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
    }
    AudioProcessConfig config;
    AudioDeviceDescriptor deviceInfo;
    GetServerPtr()->audioPolicyService_.IsDevicePlaybackSupported(config, deviceInfo);
}
} // namespace AudioStandard
} // namesapce OHOS

OHOS::AudioStandard::TestPtr g_testPtrs[OHOS::AudioStandard::TESTSIZE] = {
    OHOS::AudioStandard::AudioPolicyServiceDumpTest,
    OHOS::AudioStandard::AudioPolicyServiceDeviceTest,
    OHOS::AudioStandard::AudioPolicyServiceAccountTest,
    OHOS::AudioStandard::AudioPolicyServiceSafeVolumeTest,
    OHOS::AudioStandard::AudioPolicyServiceInterfaceTest,
    OHOS::AudioStandard::AudioDeviceConnectTest,
    OHOS::AudioStandard::AudioPolicyServiceSubscribeSafeVolumeEventFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceOnReceiveEventFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceSetAppVolumeLevelFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceSetSourceOutputStreamMuteFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetSelectedDeviceInfoFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetDistributedRoutingRoleInfoFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceNotifyCapturerAddedFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceNotifyWakeUpCapturerRemovedFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetFastStreamInfoFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceIsAbsVolumeSupportedFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetDevicesFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetPreferredInputDeviceDescriptorsFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetPreferredOutputDeviceDescInnerFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetOutputDeviceFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetInputDeviceFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetActiveOutputDeviceDescriptorFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceOnUpdateAnahsSupportFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceOnPnpDeviceStatusUpdatedFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceOnDeviceStatusUpdatedFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceUpdateA2dpOffloadFlagBySpatialServiceFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceRegisterRemoteDevStatusCallbackFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetAllSinkInputsFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceRegisterAccessibilityMonitorHelperFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceOnServiceConnectedFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceOnServiceDisconnectedFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceOnForcedDeviceSelectedFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceSetAvailableDeviceChangeCallbackFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceSetQueryClientTypeCallbackFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceSetQueryDeviceVolumeBehaviorCallbackFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceUpdateCapturerInfoWhenNoPermissionFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetCurrentCapturerChangeInfosFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceUpdateDescWhenNoBTPermissionFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetPreferredOutputStreamTypeFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetProcessDeviceInfoFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetVoipDeviceInfoFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceInitSharedVolumeFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceDynamicUnloadModuleFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetMaxRendererInstancesFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceRegisterBluetoothListenerFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceUnregisterBluetoothListenerFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceSubscribeAccessibilityConfigObserverFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceQueryEffectManagerSceneModeFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceRegisterDataObserverFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetHardwareOutputSamplingRateFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceDeviceFilterByUsageInnerFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceOffloadStartPlayingFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceOffloadStopPlayingFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceOffloadGetRenderPositionFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceNearlinkGetRenderPositionFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetAndSaveClientTypeFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceOnDeviceInfoUpdatedFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceNotifyAccountsChangedFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceLoadHdiEffectModelFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetSupportedAudioEffectPropertyFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetSupportedEffectPropertyFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetSupportedEffectPropertyFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceCheckSupportedAudioEffectPropertyFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceSetAudioEffectPropertyFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetAudioEnhancePropertyFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetAudioEffectPropertyFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetSupportedAudioEnhancePropertyFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceSetAudioEnhancePropertyFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceGetA2dpOffloadFlagFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceSetSleAudioOperationCallbackFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceNotifyCapturerRemovedFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceUpdateSpatializationSupportedFuzztest,
    OHOS::AudioStandard::AudioPolicyServiceIsDevicePlaybackSupportedFuzztest,
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