/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "audio_device_descriptor.h"
#include "istandard_client_tracker.h"
#include "audio_policy_client_proxy.h"
#include "audio_info.h"
#include "audio_stream_change_info.h"
#include "audio_stream_manager.h"
#include "audio_client_tracker_callback_service.h"
#include "audio_client_tracker_callback_listener.h"
#include <fuzzer/FuzzedDataProvider.h>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

namespace OHOS {
namespace AudioStandard {

const size_t THRESHOLD = 10;
constexpr size_t MAX_STRING_LEN = 64;
typedef void (*TestPtr)();
sptr<AudioClientTrackerCallbackService> listener = new AudioClientTrackerCallbackService();
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
        ~AudioClientTrackerFuzzTest() override = default;
        void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override {};
        void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override {};
        void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override {};
        void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override {};
        void SetLowPowerVolumeImpl(float volume) override {};
        void GetLowPowerVolumeImpl(float &volume) override {};
        void GetSingleStreamVolumeImpl(float &volume) override {};
        void SetOffloadModeImpl(int32_t state, bool isAppBack) override {};
        void UnsetOffloadModeImpl() override {};
};

std::shared_ptr<ClientTrackerCallbackListener> CreateClientTrackerCallbackListener()
{
    static std::shared_ptr<AudioClientTracker> callback = std::make_shared<AudioClientTrackerFuzzTest>();
    std::weak_ptr<AudioClientTracker> weakCallback = callback;
    listener->SetClientTrackerCallback(weakCallback);
    return std::make_shared<ClientTrackerCallbackListener>(listener);
}

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

template<typename T>
const T& PickValue(FuzzedDataProvider& fdp, const std::vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

VolumeEvent ConsumeVolumeEvent(FuzzedDataProvider& fdp)
{
    VolumeEvent volumeEvent(PickValue(fdp, g_testAudioStreamTypes), fdp.ConsumeIntegral<int32_t>(),
        fdp.ConsumeBool());
    volumeEvent.volumeDegree = fdp.ConsumeIntegral<int32_t>();
    volumeEvent.previousVolume = fdp.ConsumeIntegral<int32_t>();
    volumeEvent.volumeGroupId = fdp.ConsumeIntegral<int32_t>();
    volumeEvent.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    volumeEvent.deviceType = ConsumeEnum<DeviceType>(fdp);
    volumeEvent.volumeMode = ConsumeEnum<AudioVolumeMode>(fdp);
    volumeEvent.notifyRssWhenAccountsChange = fdp.ConsumeBool();
    volumeEvent.appUid = fdp.ConsumeIntegral<int32_t>();
    return volumeEvent;
}

StreamVolumeEvent ConsumeStreamVolumeEvent(FuzzedDataProvider& fdp)
{
    StreamVolumeEvent streamVolumeEvent;
    streamVolumeEvent.streamUsage = PickValue(fdp, g_testAudioStreamUsages);
    streamVolumeEvent.volume = fdp.ConsumeIntegral<int32_t>();
    streamVolumeEvent.updateUi = fdp.ConsumeBool();
    streamVolumeEvent.volumeGroupId = fdp.ConsumeIntegral<int32_t>();
    streamVolumeEvent.previousVolume = fdp.ConsumeIntegral<int32_t>();
    streamVolumeEvent.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    streamVolumeEvent.deviceType = ConsumeEnum<DeviceType>(fdp);
    streamVolumeEvent.volumeMode = ConsumeEnum<AudioVolumeMode>(fdp);
    return streamVolumeEvent;
}

AudioInterrupt ConsumeAudioInterrupt(FuzzedDataProvider& fdp)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamUsage = PickValue(fdp, g_testAudioStreamUsages);
    audioInterrupt.contentType = ConsumeEnum<ContentType>(fdp);
    audioInterrupt.audioFocusType.streamType = PickValue(fdp, g_testAudioStreamTypes);
    audioInterrupt.audioFocusType.sourceType = ConsumeEnum<SourceType>(fdp);
    audioInterrupt.audioFocusType.isPlay = fdp.ConsumeBool();
    audioInterrupt.streamId = fdp.ConsumeIntegral<uint32_t>();
    audioInterrupt.pauseWhenDucked = fdp.ConsumeBool();
    audioInterrupt.pid = fdp.ConsumeIntegral<int32_t>();
    audioInterrupt.uid = fdp.ConsumeIntegral<int32_t>();
    audioInterrupt.deviceTag = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioInterrupt.bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioInterrupt.mode = ConsumeEnum<InterruptMode>(fdp);
    audioInterrupt.isAudioSessionInterrupt = fdp.ConsumeBool();
    audioInterrupt.api = fdp.ConsumeIntegral<int32_t>();
    audioInterrupt.state = fdp.ConsumeIntegral<int32_t>();
    audioInterrupt.strategy = ConsumeEnum<InterruptStrategy>(fdp);
    audioInterrupt.callbackType = ConsumeEnum<InterruptEventCallbackType>(fdp);
    return audioInterrupt;
}

std::shared_ptr<AudioDeviceDescriptor> ConsumeDeviceDescriptor(FuzzedDataProvider& fdp)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    if (desc == nullptr) {
        return nullptr;
    }
    desc->deviceType_ = ConsumeEnum<DeviceType>(fdp);
    desc->deviceRole_ = ConsumeEnum<DeviceRole>(fdp);
    desc->networkId_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    desc->macAddress_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return desc;
}

AudioRendererInfo ConsumeRendererInfo(FuzzedDataProvider& fdp)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.contentType = ConsumeEnum<ContentType>(fdp);
    rendererInfo.streamUsage = PickValue(fdp, g_testAudioStreamUsages);
    rendererInfo.rendererFlags = fdp.ConsumeIntegral<int32_t>();
    rendererInfo.volumeMode = ConsumeEnum<AudioVolumeMode>(fdp);
    rendererInfo.sceneType = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    rendererInfo.spatializationEnabled = fdp.ConsumeBool();
    rendererInfo.headTrackingEnabled = fdp.ConsumeBool();
    rendererInfo.originalFlag = fdp.ConsumeIntegral<int32_t>();
    rendererInfo.pipeType = ConsumeEnum<AudioPipeType>(fdp);
    rendererInfo.samplingRate = ConsumeEnum<AudioSamplingRate>(fdp);
    rendererInfo.encodingType = fdp.ConsumeIntegral<uint8_t>();
    rendererInfo.channelLayout = fdp.ConsumeIntegral<uint64_t>();
    rendererInfo.format = ConsumeEnum<AudioSampleFormat>(fdp);
    rendererInfo.isOffloadAllowed = fdp.ConsumeBool();
    rendererInfo.playerType = ConsumeEnum<PlayerType>(fdp);
    return rendererInfo;
}

AudioCapturerInfo ConsumeCapturerInfo(FuzzedDataProvider& fdp)
{
    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = ConsumeEnum<SourceType>(fdp);
    capturerInfo.capturerFlags = fdp.ConsumeIntegral<int32_t>();
    capturerInfo.originalFlag = fdp.ConsumeIntegral<int32_t>();
    capturerInfo.pipeType = ConsumeEnum<AudioPipeType>(fdp);
    capturerInfo.samplingRate = ConsumeEnum<AudioSamplingRate>(fdp);
    capturerInfo.encodingType = fdp.ConsumeIntegral<uint8_t>();
    capturerInfo.channelLayout = fdp.ConsumeIntegral<uint64_t>();
    capturerInfo.sceneType = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    capturerInfo.recorderType = ConsumeEnum<RecorderType>(fdp);
    capturerInfo.isLoopback = fdp.ConsumeBool();
    capturerInfo.loopbackMode = ConsumeEnum<AudioLoopbackMode>(fdp);
    capturerInfo.mixWithWakeUp = fdp.ConsumeBool();
    capturerInfo.loopBackEffectEnabled = fdp.ConsumeBool();
    return capturerInfo;
}

void AudioPolicyClientProxyOnVolumeKeyEventFuzzTest(FuzzedDataProvider& fdp)
{
    VolumeEvent volumeEvent = ConsumeVolumeEvent(fdp);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnVolumeKeyEvent(volumeEvent);
}

void AudioPolicyClientProxyOnAudioFocusInfoChangeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInterrupt audioInterrupt = ConsumeAudioInterrupt(fdp);
    AudioFocuState audioFocuState = ConsumeEnum<AudioFocuState>(fdp);
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

void AudioPolicyClientProxyOnAudioFocusRequestedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInterrupt requestFocus = ConsumeAudioInterrupt(fdp);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnAudioFocusRequested(requestFocus);
}

void AudioPolicyClientProxyOnAudioFocusAbandonedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInterrupt requestFocus = ConsumeAudioInterrupt(fdp);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnAudioFocusAbandoned(requestFocus);
}

void AudioPolicyClientProxyOnActiveVolumeTypeChangedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeType volumeType = PickValue(fdp, g_testAudioStreamTypes);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnActiveVolumeTypeChanged(volumeType);
}

void AudioPolicyClientProxyOnAppVolumeChangedFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t appUid = fdp.ConsumeIntegral<int32_t>();
    VolumeEvent volumeEvent = ConsumeVolumeEvent(fdp);
    volumeEvent.appUid = appUid;
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnSelfAppVolumeChanged(appUid, volumeEvent);
    audioPolicyClientProxy->OnSystemAppVolumeChanged(appUid, volumeEvent);
}

void AudioPolicyClientProxyOnDeviceChangeFuzzTest(FuzzedDataProvider& fdp)
{
    static const vector<DeviceChangeType> testDeviceChangeTypes = {
        CONNECT,
        DISCONNECT,
    };
    DeviceChangeAction deviceChangeAction;
    deviceChangeAction.type = PickValue(fdp, testDeviceChangeTypes);
    deviceChangeAction.flag = PickValue(fdp, g_testDeviceFlags);
    deviceChangeAction.deviceDescriptors.push_back(ConsumeDeviceDescriptor(fdp));
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnDeviceChange(deviceChangeAction);
}

void AudioPolicyClientProxyOnMicrophoneBlockedFuzzTest(FuzzedDataProvider& fdp)
{
    static const vector<DeviceBlockStatus> testDeviceBlockStatuses = {
        DEVICE_UNBLOCKED,
        DEVICE_BLOCKED,
    };
    MicrophoneBlockedInfo microphoneBlockedInfo;
    microphoneBlockedInfo.blockStatus = PickValue(fdp, testDeviceBlockStatuses);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnMicrophoneBlocked(microphoneBlockedInfo);
}

void AudioPolicyClientProxyOnRingerModeUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    static const vector<AudioRingerMode> testRingerModes = {
        RINGER_MODE_SILENT,
        RINGER_MODE_VIBRATE,
        RINGER_MODE_NORMAL,
    };
    AudioRingerMode ringerMode = PickValue(fdp, testRingerModes);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnRingerModeUpdated(ringerMode);
}

void AudioPolicyClientProxyOnMicStateUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    MicStateChangeEvent micStateChangeEvent;
    micStateChangeEvent.mute = fdp.ConsumeBool();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnMicStateUpdated(micStateChangeEvent);
}

void AudioPolicyClientProxyOnPreferredOutputDeviceUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioRendererInfo rendererInfo = ConsumeRendererInfo(fdp);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc = { ConsumeDeviceDescriptor(fdp) };
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnPreferredOutputDeviceUpdated(rendererInfo, uid, desc);
}

void AudioPolicyClientProxyOnPreferredInputDeviceUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerInfo capturerInfo = ConsumeCapturerInfo(fdp);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc = { ConsumeDeviceDescriptor(fdp) };
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnPreferredInputDeviceUpdated(capturerInfo, uid, desc);
}

void AudioPolicyClientProxyOnRendererStateChangeFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    bool changeInfoNull = fdp.ConsumeBool();
    std::shared_ptr<AudioRendererChangeInfo> changeInfo = std::make_shared<AudioRendererChangeInfo>();
    if (changeInfoNull) {
        changeInfo = nullptr;
    } else {
        changeInfo->createrUID = fdp.ConsumeIntegral<int32_t>();
        changeInfo->clientUID = fdp.ConsumeIntegral<int32_t>();
        changeInfo->sessionId = fdp.ConsumeIntegral<int32_t>();
        changeInfo->callerPid = fdp.ConsumeIntegral<int32_t>();
        changeInfo->clientPid = fdp.ConsumeIntegral<int32_t>();
        changeInfo->tokenId = fdp.ConsumeIntegral<int32_t>();
        changeInfo->channelCount = fdp.ConsumeIntegral<int32_t>();
        changeInfo->rendererInfo = ConsumeRendererInfo(fdp);
        changeInfo->rendererState = ConsumeEnum<RendererState>(fdp);
        auto desc = ConsumeDeviceDescriptor(fdp);
        if (desc != nullptr) {
            changeInfo->outputDeviceInfo = *desc;
        }
        changeInfo->backMute = fdp.ConsumeBool();
        changeInfo->appVolume = fdp.ConsumeIntegral<int32_t>();
    }
    audioRendererChangeInfos.push_back(changeInfo);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnRendererStateChange(audioRendererChangeInfos);
}

void AudioPolicyClientProxyOnCapturerStateChangeFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    bool changeInfoNull = fdp.ConsumeBool();
    std::shared_ptr<AudioCapturerChangeInfo> changeInfo = std::make_shared<AudioCapturerChangeInfo>();
    if (changeInfoNull) {
        changeInfo = nullptr;
    } else {
        changeInfo->createrUID = fdp.ConsumeIntegral<int32_t>();
        changeInfo->clientUID = fdp.ConsumeIntegral<int32_t>();
        changeInfo->sessionId = fdp.ConsumeIntegral<int32_t>();
        changeInfo->callerPid = fdp.ConsumeIntegral<int32_t>();
        changeInfo->clientPid = fdp.ConsumeIntegral<int32_t>();
        changeInfo->capturerInfo = ConsumeCapturerInfo(fdp);
        changeInfo->capturerState = ConsumeEnum<CapturerState>(fdp);
        auto desc = ConsumeDeviceDescriptor(fdp);
        if (desc != nullptr) {
            changeInfo->inputDeviceInfo = *desc;
        }
        changeInfo->muted = fdp.ConsumeBool();
        changeInfo->appTokenId = fdp.ConsumeIntegral<uint32_t>();
    }
    audioCapturerChangeInfos.push_back(changeInfo);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnCapturerStateChange(audioCapturerChangeInfos);
}

void AudioPolicyClientProxyOnRendererDeviceChangeFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    AudioStreamDeviceChangeReasonExt reason = PickValue(fdp, g_testReasons);
    AudioDeviceDescriptor deviceInfo;
    AudioDeviceDescriptor preDeviceInfo;
    auto deviceInfoPtr = ConsumeDeviceDescriptor(fdp);
    auto preDeviceInfoPtr = ConsumeDeviceDescriptor(fdp);
    if (deviceInfoPtr != nullptr) {
        deviceInfo = *deviceInfoPtr;
    }
    if (preDeviceInfoPtr != nullptr) {
        preDeviceInfo = *preDeviceInfoPtr;
    }
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnRendererDeviceChange(sessionId, deviceInfo, reason, preDeviceInfo);
}

void AudioPolicyClientProxyOnRecreateRendererStreamEventFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    int32_t streamFlag = fdp.ConsumeIntegral<int32_t>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    AudioStreamDeviceChangeReasonExt reason = PickValue(fdp, g_testReasons);
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnRecreateRendererStreamEvent(sessionId, streamFlag, reason);
}

void AudioPolicyClientProxyOnRecreateCapturerStreamEventFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    int32_t streamFlag = fdp.ConsumeIntegral<int32_t>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    AudioStreamDeviceChangeReasonExt reason = PickValue(fdp, g_testReasons);
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnRecreateCapturerStreamEvent(sessionId, streamFlag, reason);
}

void AudioPolicyClientProxyOnHeadTrackingDeviceChangeFuzzTest(FuzzedDataProvider& fdp)
{
    std::unordered_map<std::string, bool> changeInfo;
    bool changeInfoBool = fdp.ConsumeBool();
    changeInfo.insert(std::make_pair(fdp.ConsumeRandomLengthString(MAX_STRING_LEN), changeInfoBool));
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    if (audioPolicyClientProxy == nullptr) {
        return;
    }
    audioPolicyClientProxy->OnHeadTrackingDeviceChange(changeInfo);
}

void AudioPolicyClientProxyOnSpatializationEnabledChangeFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    bool enabled = fdp.ConsumeBool();
    iAudioPolicyClient->OnSpatializationEnabledChange(enabled);
}

void AudioPolicyClientProxyOnSpatializationEnabledChangeForAnyDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = ConsumeDeviceDescriptor(fdp);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    bool enabled = fdp.ConsumeBool();
    iAudioPolicyClient->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled);
}

void AudioPolicyClientProxyOnSpatializationEnabledChangeForCurrentDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    bool enabled = fdp.ConsumeBool();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnSpatializationEnabledChangeForCurrentDevice(enabled);
}

void AudioPolicyClientProxyOnHeadTrackingEnabledChangeFuzzTest(FuzzedDataProvider& fdp)
{
    bool enabled = fdp.ConsumeBool();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnHeadTrackingEnabledChange(enabled);
}

void AudioPolicyClientProxyOnAudioSceneChangeFuzzTest(FuzzedDataProvider& fdp)
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
    AudioScene audioScene = PickValue(fdp, testAudioScenes);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnAudioSceneChange(audioScene);
}

void AudioPolicyClientProxyOnHeadTrackingEnabledChangeForAnyDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    bool enabled = fdp.ConsumeBool();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = ConsumeDeviceDescriptor(fdp);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnHeadTrackingEnabledChangeForAnyDevice(deviceDescriptor, enabled);
}

void AudioPolicyClientProxyOnNnStateChangeFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t state = fdp.ConsumeIntegral<int32_t>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnNnStateChange(state);
}

void AudioPolicyClientProxyOnAudioSessionDeactiveFuzzTest(FuzzedDataProvider& fdp)
{
    static const vector<AudioSessionDeactiveReason> testDeactiveReasons = {
        AudioSessionDeactiveReason::LOW_PRIORITY,
        AudioSessionDeactiveReason::TIMEOUT,
    };
    AudioSessionDeactiveEvent deactiveEvent;
    if (testDeactiveReasons.size() == 0) {
        return;
    }
    deactiveEvent.deactiveReason = PickValue(fdp, testDeactiveReasons);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnAudioSessionDeactive(static_cast<int32_t>(deactiveEvent.deactiveReason));
}

void AudioPolicyClientProxyOnFormatUnsupportedErrorFuzzTest(FuzzedDataProvider& fdp)
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
    AudioErrors errorCode = PickValue(fdp, errorCodes);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnFormatUnsupportedError(errorCode);
}

void AudioPolicyClientProxyOnStreamVolumeChangeFuzzTest(FuzzedDataProvider& fdp)
{
    StreamVolumeEvent streamVolumeEvent = ConsumeStreamVolumeEvent(fdp);
    VolumeEvent volumeEvent = ConsumeVolumeEvent(fdp);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnStreamVolumeChange(streamVolumeEvent);
    iAudioPolicyClient->OnSystemVolumeChange(volumeEvent);
}

void AudioPolicyClientProxyOnSystemVolumeChangeByFilterFuzzTest(FuzzedDataProvider& fdp)
{
    SystemVolumeFilter filter;
    filter.uid = fdp.ConsumeIntegral<int32_t>();
    VolumeEvent volumeEvent = ConsumeVolumeEvent(fdp);
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnSystemVolumeChangeByFilter(filter, volumeEvent);
}

void AudioPolicyClientProxyOnVolumeLimitExceededFuzzTest(FuzzedDataProvider& fdp)
{
    VolumeLimitExceedEvent event;
    event.volumeType = PickValue(fdp, g_testAudioStreamTypes);
    event.currentVolume = fdp.ConsumeIntegral<int32_t>();
    event.volumeThreshold = fdp.ConsumeIntegral<int32_t>();
    event.uid = fdp.ConsumeIntegral<int32_t>();
    sptr<IRemoteObject> impl = new RemoteObjectFuzzTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    if (iAudioPolicyClient == nullptr) {
        return;
    }
    iAudioPolicyClient->OnVolumeLimitExceeded(event);
}

void AudioClientTrackerCallbackProxyMuteStreamImplFuzzTest(FuzzedDataProvider& fdp)
{
    auto clientTrackerCallbackListener = CreateClientTrackerCallbackListener();
    if (clientTrackerCallbackListener == nullptr || g_testStreamSetStates.size() == 0
        || g_testAudioStreamUsages.size() == 0) {
        return;
    }
    StreamSetStateEventInternal streamSetStateEventInternal;
    streamSetStateEventInternal.streamSetState =
        PickValue(fdp, g_testStreamSetStates);
    streamSetStateEventInternal.streamUsage =
        PickValue(fdp, g_testAudioStreamUsages);
    clientTrackerCallbackListener->MuteStreamImpl(streamSetStateEventInternal);
}

void AudioClientTrackerCallbackProxyUnmuteStreamImplFuzzTest(FuzzedDataProvider& fdp)
{
    auto clientTrackerCallbackListener = CreateClientTrackerCallbackListener();
    if (clientTrackerCallbackListener == nullptr || g_testStreamSetStates.size() == 0
        || g_testAudioStreamUsages.size() == 0) {
        return;
    }
    StreamSetStateEventInternal streamSetStateEventInternal;
    streamSetStateEventInternal.streamSetState =
        PickValue(fdp, g_testStreamSetStates);
    streamSetStateEventInternal.streamUsage =
        PickValue(fdp, g_testAudioStreamUsages);
    clientTrackerCallbackListener->UnmuteStreamImpl(streamSetStateEventInternal);
}

void AudioClientTrackerCallbackProxyPausedStreamImplFuzzTest(FuzzedDataProvider& fdp)
{
    auto clientTrackerCallbackListener = CreateClientTrackerCallbackListener();
    if (clientTrackerCallbackListener == nullptr || g_testStreamSetStates.size() == 0
        || g_testAudioStreamUsages.size() == 0) {
        return;
    }
    StreamSetStateEventInternal streamSetStateEventInternal;
    streamSetStateEventInternal.streamSetState =
        PickValue(fdp, g_testStreamSetStates);
    streamSetStateEventInternal.streamUsage =
        PickValue(fdp, g_testAudioStreamUsages);
    clientTrackerCallbackListener->PausedStreamImpl(streamSetStateEventInternal);
}

void AudioClientTrackerCallbackProxyResumeStreamImplFuzzTest(FuzzedDataProvider& fdp)
{
    auto clientTrackerCallbackListener = CreateClientTrackerCallbackListener();
    if (clientTrackerCallbackListener == nullptr || g_testStreamSetStates.size() == 0
        || g_testAudioStreamUsages.size() == 0) {
        return;
    }
    StreamSetStateEventInternal streamSetStateEventInternal;
    streamSetStateEventInternal.streamSetState =
        PickValue(fdp, g_testStreamSetStates);
    streamSetStateEventInternal.streamUsage =
        PickValue(fdp, g_testAudioStreamUsages);
    clientTrackerCallbackListener->ResumeStreamImpl(streamSetStateEventInternal);
}

void AudioClientTrackerCallbackProxySetLowPowerVolumeImplFuzzTest(FuzzedDataProvider& fdp)
{
    auto clientTrackerCallbackListener = CreateClientTrackerCallbackListener();
    if (clientTrackerCallbackListener == nullptr) {
        return;
    }
    float volume = fdp.ConsumeFloatingPoint<float>();
    clientTrackerCallbackListener->SetLowPowerVolumeImpl(volume);
}

void AudioClientTrackerCallbackProxyGetLowPowerVolumeImplFuzzTest(FuzzedDataProvider& fdp)
{
    auto clientTrackerCallbackListener = CreateClientTrackerCallbackListener();
    if (clientTrackerCallbackListener == nullptr) {
        return;
    }
    float volume = fdp.ConsumeFloatingPoint<float>();
    clientTrackerCallbackListener->GetLowPowerVolumeImpl(volume);
}

void AudioClientTrackerCallbackProxyGetSingleStreamVolumeImplFuzzTest(FuzzedDataProvider& fdp)
{
    auto clientTrackerCallbackListener = CreateClientTrackerCallbackListener();
    if (clientTrackerCallbackListener == nullptr) {
        return;
    }
    float volume = fdp.ConsumeFloatingPoint<float>();
    clientTrackerCallbackListener->GetSingleStreamVolumeImpl(volume);
}

void AudioClientTrackerCallbackProxySetOffloadModeImplFuzzTest(FuzzedDataProvider& fdp)
{
    auto clientTrackerCallbackListener = CreateClientTrackerCallbackListener();
    if (clientTrackerCallbackListener == nullptr) {
        return;
    }
    int32_t state = fdp.ConsumeIntegral<int32_t>();
    bool isAppBack = fdp.ConsumeBool();
    clientTrackerCallbackListener->SetOffloadModeImpl(state, isAppBack);
}

void AudioClientTrackerCallbackProxyUnsetOffloadModeImplFuzzTest(FuzzedDataProvider& fdp)
{
    auto clientTrackerCallbackListener = CreateClientTrackerCallbackListener();
    if (clientTrackerCallbackListener == nullptr) {
        return;
    }
    clientTrackerCallbackListener->UnsetOffloadModeImpl();
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
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
        AudioPolicyClientProxyOnSystemVolumeChangeByFilterFuzzTest,
        AudioPolicyClientProxyOnVolumeLimitExceededFuzzTest,
        AudioClientTrackerCallbackProxyMuteStreamImplFuzzTest,
        AudioClientTrackerCallbackProxyUnmuteStreamImplFuzzTest,
        AudioClientTrackerCallbackProxyPausedStreamImplFuzzTest,
        AudioClientTrackerCallbackProxyResumeStreamImplFuzzTest,
        AudioClientTrackerCallbackProxySetLowPowerVolumeImplFuzzTest,
        AudioClientTrackerCallbackProxyGetLowPowerVolumeImplFuzzTest,
        AudioClientTrackerCallbackProxyGetSingleStreamVolumeImplFuzzTest,
        AudioClientTrackerCallbackProxySetOffloadModeImplFuzzTest,
        AudioClientTrackerCallbackProxyUnsetOffloadModeImplFuzzTest,
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    OHOS::AudioStandard::Init();
    return 0;
}
