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

#include <securec.h>
#include "audio_log.h"
#include "audio_policy_client_proxy.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)();

class RemoteObjectFuzzTestStub : public IRemoteObject {
public:
    RemoteObjectFuzzTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectFuzzTestStub");
};

const vector<AudioStreamType> g_testAudioStreamTypes = {
    STREAM_DEFAULT,
    STREAM_VOICE_CALL,
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_MEDIA,
    STREAM_VOICE_ASSISTANT,
    STREAM_SYSTEM,
    STREAM_ALARM,
    STREAM_NOTIFICATION,
    STREAM_BLUETOOTH_SCO,
    STREAM_ENFORCED_AUDIBLE,
    STREAM_DTMF,
    STREAM_TTS,
    STREAM_ACCESSIBILITY,
    STREAM_RECORDING,
    STREAM_MOVIE,
    STREAM_GAME,
    STREAM_SPEECH,
    STREAM_SYSTEM_ENFORCED,
    STREAM_ULTRASONIC,
    STREAM_WAKEUP,
    STREAM_VOICE_MESSAGE,
    STREAM_NAVIGATION,
    STREAM_INTERNAL_FORCE_STOP,
    STREAM_SOURCE_VOICE_CALL,
    STREAM_VOICE_COMMUNICATION,
    STREAM_VOICE_RING,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_CAMCORDER,
    STREAM_APP,
    STREAM_TYPE_MAX,
    STREAM_ALL,
};

const vector<DeviceFlag> g_testDeviceFlags = {
    NONE_DEVICES_FLAG,
    OUTPUT_DEVICES_FLAG,
    INPUT_DEVICES_FLAG,
    ALL_DEVICES_FLAG,
    DISTRIBUTED_OUTPUT_DEVICES_FLAG,
    DISTRIBUTED_INPUT_DEVICES_FLAG,
    ALL_DISTRIBUTED_DEVICES_FLAG,
    ALL_L_D_DEVICES_FLAG,
    DEVICE_FLAG_MAX
};

const vector<AudioStreamDeviceChangeReason> g_testReasons = {
    AudioStreamDeviceChangeReason::UNKNOWN,
    AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE,
    AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE,
    AudioStreamDeviceChangeReason::OVERRODE,
};

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

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

void AudioPolicyClientProxyOnVolumeKeyEventFuzzTest()
{
    VolumeEvent volumeEvent;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnVolumeKeyEvent(volumeEvent);
}

void AudioPolicyClientProxyOnAudioFocusInfoChangeFuzzTest()
{
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    focusInfoList.emplace_back(make_pair(audioInterrupt, audioFocuState));

    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnAudioFocusInfoChange(focusInfoList);
}

void AudioPolicyClientProxyOnAudioFocusRequestedFuzzTest()
{
    AudioInterrupt requestFocus;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnAudioFocusRequested(requestFocus);
}

void AudioPolicyClientProxyOnAudioFocusAbandonedFuzzTest()
{
    AudioInterrupt requestFocus;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnAudioFocusAbandoned(requestFocus);
}

void AudioPolicyClientProxyOnActiveVolumeTypeChangedFuzzTest()
{
    AudioVolumeType volumeType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnActiveVolumeTypeChanged(volumeType);
}

void AudioPolicyClientProxyOnAppVolumeChangedFuzzTest()
{
    AudioVolumeType volType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    int32_t volLevel = GetData<int32_t>();
    bool isUiUpdated = GetData<bool>();
    VolumeEvent volumeEvent(volType, volLevel, isUiUpdated);
    int32_t appUid = GetData<int32_t>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnAppVolumeChanged(appUid, volumeEvent);
}

void AudioPolicyClientProxyOnDeviceChangeFuzzTest()
{
    static const vector<DeviceChangeType> testDeviceChangeTypes = {
        CONNECT,
        DISCONNECT,
    };
    DeviceChangeAction deviceChangeAction;
    deviceChangeAction.type = testDeviceChangeTypes[GetData<uint32_t>() % testDeviceChangeTypes.size()];
    deviceChangeAction.flag = g_testDeviceFlags[GetData<uint32_t>() % g_testDeviceFlags.size()];
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnDeviceChange(deviceChangeAction);
}

void AudioPolicyClientProxyOnMicrophoneBlockedFuzzTest()
{
    static const vector<DeviceBlockStatus> testDeviceBlockStatuses = {
        DEVICE_UNBLOCKED,
        DEVICE_BLOCKED,
    };
    MicrophoneBlockedInfo microphoneBlockedInfo;
    microphoneBlockedInfo.blockStatus = testDeviceBlockStatuses[GetData<uint32_t>() % testDeviceBlockStatuses.size()];
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnMicrophoneBlocked(microphoneBlockedInfo);
}

void AudioPolicyClientProxyOnRingerModeUpdatedFuzzTest()
{
    static const vector<AudioRingerMode> testRingerModes = {
        RINGER_MODE_SILENT,
        RINGER_MODE_VIBRATE,
        RINGER_MODE_NORMAL,
    };
    AudioRingerMode ringerMode = testRingerModes[GetData<uint32_t>() % testRingerModes.size()];
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnRingerModeUpdated(ringerMode);
}

void AudioPolicyClientProxyOnMicStateUpdatedFuzzTest()
{
    MicStateChangeEvent micStateChangeEvent;
    micStateChangeEvent.mute = GetData<bool>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnMicStateUpdated(micStateChangeEvent);
}

void AudioPolicyClientProxyOnPreferredOutputDeviceUpdatedFuzzTest()
{
    AudioRendererInfo rendererInfo;
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnPreferredOutputDeviceUpdated(rendererInfo, desc);
}

void AudioPolicyClientProxyOnPreferredInputDeviceUpdatedFuzzTest()
{
    AudioCapturerInfo capturerInfo;
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnPreferredInputDeviceUpdated(capturerInfo, desc);
}

void AudioPolicyClientProxyOnRendererStateChangeFuzzTest()
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    bool changeInfoNull = GetData<bool>();
    std::shared_ptr<AudioRendererChangeInfo> changeInfo = std::make_shared<AudioRendererChangeInfo>();
    if (changeInfoNull) {
        changeInfo = nullptr;
    }
    audioRendererChangeInfos.push_back(changeInfo);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnRendererStateChange(audioRendererChangeInfos);
}

void AudioPolicyClientProxyOnCapturerStateChangeFuzzTest()
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    bool changeInfoNull = GetData<bool>();
    std::shared_ptr<AudioCapturerChangeInfo> changeInfo = std::make_shared<AudioCapturerChangeInfo>();
    if (changeInfoNull) {
        changeInfo = nullptr;
    }
    audioCapturerChangeInfos.push_back(changeInfo);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnCapturerStateChange(audioCapturerChangeInfos);
}

void AudioPolicyClientProxyOnRendererDeviceChangeFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    AudioStreamDeviceChangeReasonExt reason = g_testReasons[GetData<uint32_t>() % g_testReasons.size()];
    AudioDeviceDescriptor deviceInfo;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnRendererDeviceChange(sessionId, deviceInfo, reason);
}

void AudioPolicyClientProxyOnRecreateRendererStreamEventFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    int32_t streamFlag = GetData<int32_t>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    AudioStreamDeviceChangeReasonExt reason = g_testReasons[GetData<uint32_t>() % g_testReasons.size()];
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnRecreateRendererStreamEvent(sessionId, streamFlag, reason);
}

void AudioPolicyClientProxyOnRecreateCapturerStreamEventFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    int32_t streamFlag = GetData<int32_t>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    AudioStreamDeviceChangeReasonExt reason = g_testReasons[GetData<uint32_t>() % g_testReasons.size()];
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnRecreateCapturerStreamEvent(sessionId, streamFlag, reason);
}

void AudioPolicyClientProxyOnHeadTrackingDeviceChangeFuzzTest()
{
    std::unordered_map<std::string, bool> changeInfo;
    bool changeInfoBool = GetData<bool>();
    changeInfo.insert(std::make_pair("testKey", changeInfoBool));
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnHeadTrackingDeviceChange(changeInfo);
}

void AudioPolicyClientProxyOnSpatializationEnabledChangeFuzzTest()
{
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    iAudioPolicyClient->hasSystemPermission_ = GetData<bool>();
    bool enabled = GetData<bool>();
    iAudioPolicyClient->OnSpatializationEnabledChange(enabled);
}

void AudioPolicyClientProxyOnSpatializationEnabledChangeForAnyDeviceFuzzTest()
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    iAudioPolicyClient->hasSystemPermission_ = GetData<bool>();
    bool enabled = GetData<bool>();
    iAudioPolicyClient->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled);
}

TestPtr g_testPtrs[] = {
    AudioPolicyClientProxyOnVolumeKeyEventFuzzTest,
    AudioPolicyClientProxyOnAudioFocusInfoChangeFuzzTest,
    AudioPolicyClientProxyOnAudioFocusRequestedFuzzTest,
    AudioPolicyClientProxyOnAudioFocusAbandonedFuzzTest,
    AudioPolicyClientProxyOnActiveVolumeTypeChangedFuzzTest,
    AudioPolicyClientProxyOnAppVolumeChangedFuzzTest,
    AudioPolicyClientProxyOnDeviceChangeFuzzTest,
    AudioPolicyClientProxyOnMicrophoneBlockedFuzzTest,
    AudioPolicyClientProxyOnRingerModeUpdatedFuzzTest,
    AudioPolicyClientProxyOnMicStateUpdatedFuzzTest,
    AudioPolicyClientProxyOnPreferredOutputDeviceUpdatedFuzzTest,
    AudioPolicyClientProxyOnPreferredInputDeviceUpdatedFuzzTest,
    AudioPolicyClientProxyOnRendererStateChangeFuzzTest,
    AudioPolicyClientProxyOnCapturerStateChangeFuzzTest,
    AudioPolicyClientProxyOnRendererDeviceChangeFuzzTest,
    AudioPolicyClientProxyOnRecreateRendererStreamEventFuzzTest,
    AudioPolicyClientProxyOnRecreateCapturerStreamEventFuzzTest,
    AudioPolicyClientProxyOnHeadTrackingDeviceChangeFuzzTest,
    AudioPolicyClientProxyOnSpatializationEnabledChangeFuzzTest,
    AudioPolicyClientProxyOnSpatializationEnabledChangeForAnyDeviceFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testPtrs);
    if (len > 0) {
        g_testPtrs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }
    return true;
}

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}