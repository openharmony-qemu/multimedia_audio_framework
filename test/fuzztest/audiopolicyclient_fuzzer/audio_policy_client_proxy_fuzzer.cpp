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
#include "audio_stream_manager.h"
#include "audio_client_tracker_callback_service.h"
#include "audio_client_tracker_callback_listener.h"

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

class AudioClientTrackerFuzzTest : public AudioClientTracker {
    public:
        virtual ~AudioClientTrackerFuzzTest() = default;
        virtual void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void SetLowPowerVolumeImpl(float volume) {};
        virtual void GetLowPowerVolumeImpl(float &volume) {};
        virtual void GetSingleStreamVolumeImpl(float &volume) {};
        virtual void SetOffloadModeImpl(int32_t state, bool isAppBack) {};
        virtual void UnsetOffloadModeImpl() {};
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

const vector<StreamSetState> g_testStreamSetStates = {
    STREAM_PAUSE,
    STREAM_RESUME,
    STREAM_MUTE,
    STREAM_UNMUTE,
};

const vector<StreamUsage> g_testAudioStreamUsages = {
    STREAM_USAGE_INVALID,
    STREAM_USAGE_UNKNOWN,
    STREAM_USAGE_MEDIA,
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_VOICE_ASSISTANT,
    STREAM_USAGE_ALARM,
    STREAM_USAGE_VOICE_MESSAGE,
    STREAM_USAGE_NOTIFICATION_RINGTONE,
    STREAM_USAGE_RINGTONE,
    STREAM_USAGE_NOTIFICATION,
    STREAM_USAGE_ACCESSIBILITY,
    STREAM_USAGE_SYSTEM,
    STREAM_USAGE_MOVIE,
    STREAM_USAGE_GAME,
    STREAM_USAGE_AUDIOBOOK,
    STREAM_USAGE_NAVIGATION,
    STREAM_USAGE_DTMF,
    STREAM_USAGE_ENFORCED_TONE,
    STREAM_USAGE_ULTRASONIC,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_RANGING,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
    STREAM_USAGE_VOICE_RINGTONE,
    STREAM_USAGE_VOICE_CALL_ASSISTANT,
    STREAM_USAGE_MAX,
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
    AudioFocuState audioFocuState = AudioFocuState::ACTIVE;
    std::vector<std::map<AudioInterrupt, int32_t>> focusInfoList;
    std::map<AudioInterrupt, int32_t> interruptMap;
    interruptMap[audioInterrupt] = audioFocuState;
    focusInfoList.emplace_back(interruptMap);

    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnAudioFocusInfoChange(focusInfoList);
}

void AudioPolicyClientProxyOnAudioFocusRequestedFuzzTest()
{
    AudioInterrupt requestFocus;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnAudioFocusRequested(requestFocus);
}

void AudioPolicyClientProxyOnAudioFocusAbandonedFuzzTest()
{
    AudioInterrupt requestFocus;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnAudioFocusAbandoned(requestFocus);
}

void AudioPolicyClientProxyOnActiveVolumeTypeChangedFuzzTest()
{
    AudioVolumeType volumeType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
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
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
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
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
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
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
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
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnRingerModeUpdated(ringerMode);
}

void AudioPolicyClientProxyOnMicStateUpdatedFuzzTest()
{
    MicStateChangeEvent micStateChangeEvent;
    micStateChangeEvent.mute = GetData<bool>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnMicStateUpdated(micStateChangeEvent);
}

void AudioPolicyClientProxyOnPreferredOutputDeviceUpdatedFuzzTest()
{
    AudioRendererInfo rendererInfo;
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnPreferredOutputDeviceUpdated(rendererInfo, desc);
}

void AudioPolicyClientProxyOnPreferredInputDeviceUpdatedFuzzTest()
{
    AudioCapturerInfo capturerInfo;
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
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
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
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
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnCapturerStateChange(audioCapturerChangeInfos);
}

void AudioPolicyClientProxyOnRendererDeviceChangeFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    AudioStreamDeviceChangeReasonExt reason = g_testReasons[GetData<uint32_t>() % g_testReasons.size()];
    AudioDeviceDescriptor deviceInfo;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnRendererDeviceChange(sessionId, deviceInfo, reason);
}

void AudioPolicyClientProxyOnRecreateRendererStreamEventFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    int32_t streamFlag = GetData<int32_t>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    AudioStreamDeviceChangeReasonExt reason = g_testReasons[GetData<uint32_t>() % g_testReasons.size()];
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnRecreateRendererStreamEvent(sessionId, streamFlag, reason);
}

void AudioPolicyClientProxyOnRecreateCapturerStreamEventFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    int32_t streamFlag = GetData<int32_t>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    AudioStreamDeviceChangeReasonExt reason = g_testReasons[GetData<uint32_t>() % g_testReasons.size()];
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnRecreateCapturerStreamEvent(sessionId, streamFlag, reason);
}

void AudioPolicyClientProxyOnHeadTrackingDeviceChangeFuzzTest()
{
    std::unordered_map<std::string, bool> changeInfo;
    bool changeInfoBool = GetData<bool>();
    changeInfo.insert(std::make_pair("testKey", changeInfoBool));
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnHeadTrackingDeviceChange(changeInfo);
}

void AudioPolicyClientProxyOnSpatializationEnabledChangeFuzzTest()
{
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    bool enabled = GetData<bool>();
    iAudioPolicyClient->OnSpatializationEnabledChange(enabled);
}

void AudioPolicyClientProxyOnSpatializationEnabledChangeForAnyDeviceFuzzTest()
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    bool enabled = GetData<bool>();
    iAudioPolicyClient->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled);
}

void AudioPolicyClientProxyOnSpatializationEnabledChangeForCurrentDeviceFuzzTest()
{
    bool enabled = GetData<bool>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnSpatializationEnabledChangeForCurrentDevice(enabled);
}

void AudioPolicyClientProxyOnHeadTrackingEnabledChangeFuzzTest()
{
    bool enabled = GetData<bool>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnHeadTrackingEnabledChange(enabled);
}

void AudioPolicyClientProxyOnAudioSceneChangeFuzzTest()
{
    static const vector<AudioScene> testAudioScenes = {
        AUDIO_SCENE_INVALID,
        AUDIO_SCENE_DEFAULT,
        AUDIO_SCENE_RINGING,
        AUDIO_SCENE_PHONE_CALL,
        AUDIO_SCENE_PHONE_CHAT,
        AUDIO_SCENE_CALL_START,
        AUDIO_SCENE_CALL_END,
        AUDIO_SCENE_VOICE_RINGING,
        AUDIO_SCENE_MAX,
    };
    if (testAudioScenes.size() == 0) {
        return;
    }
    AudioScene audioScene = testAudioScenes[GetData<uint32_t>() % testAudioScenes.size()];
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnAudioSceneChange(audioScene);
}

void AudioPolicyClientProxyOnHeadTrackingEnabledChangeForAnyDeviceFuzzTest()
{
    bool enabled = GetData<bool>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnHeadTrackingEnabledChangeForAnyDevice(deviceDescriptor, enabled);
}

void AudioPolicyClientProxyOnNnStateChangeFuzzTest()
{
    int32_t state = GetData<int32_t>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnNnStateChange(state);
}

void AudioPolicyClientProxyOnAudioSessionDeactiveFuzzTest()
{
    static const vector<AudioSessionDeactiveReason> testDeactiveReasons = {
        AudioSessionDeactiveReason::LOW_PRIORITY,
        AudioSessionDeactiveReason::TIMEOUT,
    };
    AudioSessionDeactiveEvent deactiveEvent;
    if (testDeactiveReasons.size() == 0) {
        return;
    }
    deactiveEvent.deactiveReason = testDeactiveReasons[GetData<uint32_t>() % testDeactiveReasons.size()];
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnAudioSessionDeactive(static_cast<int32_t>(deactiveEvent.deactiveReason));
}

void AudioPolicyClientProxyOnFormatUnsupportedErrorFuzzTest()
{
    static const vector<AudioErrors> errorCodes = {
        ERROR_INVALID_PARAM,
        ERROR_NO_MEMORY,
        ERROR_ILLEGAL_STATE,
        ERROR_UNSUPPORTED,
        ERROR_TIMEOUT,
        ERROR_UNSUPPORTED_FORMAT,
        ERROR_STREAM_LIMIT,
        ERROR_SYSTEM,
    };
    if (errorCodes.size() == 0) {
        return;
    }
    AudioErrors errorCode = errorCodes[GetData<uint32_t>() % errorCodes.size()];
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnFormatUnsupportedError(errorCode);
}

void AudioPolicyClientProxyOnStreamVolumeChangeFuzzTest()
{
    StreamVolumeEvent streamVolumeEvent;
    VolumeEvent volumeEvent;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnStreamVolumeChange(streamVolumeEvent);
    iAudioPolicyClient->OnSystemVolumeChange(volumeEvent);
}

void AudioClientTrackerCallbackProxyMuteStreamImplFuzzTest()
{
    sptr<AudioClientTrackerCallbackService> listener = new AudioClientTrackerCallbackService();
    std::weak_ptr<AudioClientTrackerFuzzTest> callback = std::make_shared<AudioClientTrackerFuzzTest>();
    listener->SetClientTrackerCallback(callback);
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    if (clientTrackerCallbackListener == nullptr || g_testStreamSetStates.size() == 0
        || g_testAudioStreamUsages.size() == 0) {
        return;
    }
    StreamSetStateEventInternal streamSetStateEventInternal;
    streamSetStateEventInternal.streamSetState =
        g_testStreamSetStates[GetData<uint32_t>() % g_testStreamSetStates.size()];
    streamSetStateEventInternal.streamUsage =
        g_testAudioStreamUsages[GetData<uint32_t>() % g_testAudioStreamUsages.size()];
    clientTrackerCallbackListener->MuteStreamImpl(streamSetStateEventInternal);
}

void AudioClientTrackerCallbackProxyUnmuteStreamImplFuzzTest()
{
    sptr<AudioClientTrackerCallbackService> listener = new AudioClientTrackerCallbackService();

    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    if (clientTrackerCallbackListener == nullptr || g_testStreamSetStates.size() == 0
        || g_testAudioStreamUsages.size() == 0) {
        return;
    }
    StreamSetStateEventInternal streamSetStateEventInternal;
    streamSetStateEventInternal.streamSetState =
        g_testStreamSetStates[GetData<uint32_t>() % g_testStreamSetStates.size()];
    streamSetStateEventInternal.streamUsage =
        g_testAudioStreamUsages[GetData<uint32_t>() % g_testAudioStreamUsages.size()];
    clientTrackerCallbackListener->UnmuteStreamImpl(streamSetStateEventInternal);
}

void AudioClientTrackerCallbackProxyPausedStreamImplFuzzTest()
{
    sptr<AudioClientTrackerCallbackService> listener = new AudioClientTrackerCallbackService();
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    if (clientTrackerCallbackListener == nullptr || g_testStreamSetStates.size() == 0
        || g_testAudioStreamUsages.size() == 0) {
        return;
    }
    StreamSetStateEventInternal streamSetStateEventInternal;
    streamSetStateEventInternal.streamSetState =
        g_testStreamSetStates[GetData<uint32_t>() % g_testStreamSetStates.size()];
    streamSetStateEventInternal.streamUsage =
        g_testAudioStreamUsages[GetData<uint32_t>() % g_testAudioStreamUsages.size()];
    clientTrackerCallbackListener->PausedStreamImpl(streamSetStateEventInternal);
}

void AudioClientTrackerCallbackProxyResumeStreamImplFuzzTest()
{
    sptr<AudioClientTrackerCallbackService> listener = new AudioClientTrackerCallbackService();
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    if (clientTrackerCallbackListener == nullptr || g_testStreamSetStates.size() == 0
        || g_testAudioStreamUsages.size() == 0) {
        return;
    }
    StreamSetStateEventInternal streamSetStateEventInternal;
    streamSetStateEventInternal.streamSetState =
        g_testStreamSetStates[GetData<uint32_t>() % g_testStreamSetStates.size()];
    streamSetStateEventInternal.streamUsage =
        g_testAudioStreamUsages[GetData<uint32_t>() % g_testAudioStreamUsages.size()];
    clientTrackerCallbackListener->ResumeStreamImpl(streamSetStateEventInternal);
}

void AudioClientTrackerCallbackProxySetLowPowerVolumeImplFuzzTest()
{
    sptr<AudioClientTrackerCallbackService> listener = new AudioClientTrackerCallbackService();
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    if (clientTrackerCallbackListener == nullptr) {
        return;
    }
    float volume = GetData<float>();
    clientTrackerCallbackListener->SetLowPowerVolumeImpl(volume);
}

void AudioClientTrackerCallbackProxyGetLowPowerVolumeImplFuzzTest()
{
    sptr<AudioClientTrackerCallbackService> listener = new AudioClientTrackerCallbackService();
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    if (clientTrackerCallbackListener == nullptr) {
        return;
    }
    float volume = GetData<float>();
    clientTrackerCallbackListener->GetLowPowerVolumeImpl(volume);
}

void AudioClientTrackerCallbackProxyGetSingleStreamVolumeImplFuzzTest()
{
    sptr<AudioClientTrackerCallbackService> listener = new AudioClientTrackerCallbackService();
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    if (clientTrackerCallbackListener == nullptr) {
        return;
    }
    float volume = GetData<float>();
    clientTrackerCallbackListener->GetSingleStreamVolumeImpl(volume);
}

void AudioClientTrackerCallbackProxySetOffloadModeImplFuzzTest()
{
    sptr<AudioClientTrackerCallbackService> listener = new AudioClientTrackerCallbackService();
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    if (clientTrackerCallbackListener == nullptr) {
        return;
    }
    int32_t state = GetData<int32_t>();
    bool isAppBack = GetData<bool>();
    clientTrackerCallbackListener->SetOffloadModeImpl(state, isAppBack);
}

void AudioClientTrackerCallbackProxyUnsetOffloadModeImplFuzzTest()
{
    sptr<AudioClientTrackerCallbackService> listener = new AudioClientTrackerCallbackService();
    auto clientTrackerCallbackListener = std::make_shared<ClientTrackerCallbackListener>(listener);
    if (clientTrackerCallbackListener == nullptr) {
        return;
    }
    clientTrackerCallbackListener->UnsetOffloadModeImpl();
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
    AudioPolicyClientProxyOnSpatializationEnabledChangeForCurrentDeviceFuzzTest,
    AudioPolicyClientProxyOnHeadTrackingEnabledChangeFuzzTest,
    AudioPolicyClientProxyOnAudioSceneChangeFuzzTest,
    AudioPolicyClientProxyOnHeadTrackingEnabledChangeForAnyDeviceFuzzTest,
    AudioPolicyClientProxyOnNnStateChangeFuzzTest,
    AudioPolicyClientProxyOnAudioSessionDeactiveFuzzTest,
    AudioPolicyClientProxyOnFormatUnsupportedErrorFuzzTest,
    AudioPolicyClientProxyOnStreamVolumeChangeFuzzTest,
    AudioClientTrackerCallbackProxyMuteStreamImplFuzzTest,
    AudioClientTrackerCallbackProxyUnmuteStreamImplFuzzTest,
    AudioClientTrackerCallbackProxyPausedStreamImplFuzzTest,
    AudioClientTrackerCallbackProxyResumeStreamImplFuzzTest,
    AudioClientTrackerCallbackProxySetLowPowerVolumeImplFuzzTest,
    AudioClientTrackerCallbackProxyGetSingleStreamVolumeImplFuzzTest,
    AudioClientTrackerCallbackProxySetOffloadModeImplFuzzTest,
    AudioClientTrackerCallbackProxyUnsetOffloadModeImplFuzzTest,
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