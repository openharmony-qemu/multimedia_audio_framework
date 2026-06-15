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

#include <iremote_stub.h>
#include "audio_log.h"
#include "sle_audio_device_manager.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
typedef void (*TestPtr)(FuzzedDataProvider&);

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

template<typename T>
T PickValue(FuzzedDataProvider& fdp, const vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

AudioDeviceDescriptor BuildDeviceDescriptor(FuzzedDataProvider& fdp, DeviceType deviceType, const std::string &addr)
{
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.deviceType_ = deviceType;
    deviceDesc.connectState_ = fdp.ConsumeBool() ? CONNECTED : SUSPEND_CONNECTED;
    deviceDesc.macAddress_ = ConsumeString(fdp, addr);
    deviceDesc.mediaVolume_ = fdp.ConsumeIntegral<int32_t>();
    deviceDesc.callVolume_ = fdp.ConsumeIntegral<int32_t>();
    return deviceDesc;
}

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

const vector<SourceType> g_testSourceTypes = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_REMOTE_CAST,
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
    SOURCE_TYPE_EC,
    SOURCE_TYPE_MIC_REF,
    SOURCE_TYPE_LIVE,
    SOURCE_TYPE_MAX,
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

const vector<SleAudioStreamType> g_testSleAudioStreamTypes = {
    SLE_AUDIO_STREAM_NONE,
    SLE_AUDIO_STREAM_UNDEFINED,
    SLE_AUDIO_STREAM_MUSIC,
    SLE_AUDIO_STREAM_VOICE_CALL,
    SLE_AUDIO_STREAM_VOICE_ASSISTANT,
    SLE_AUDIO_STREAM_RING,
    SLE_AUDIO_STREAM_VOIP,
    SLE_AUDIO_STREAM_GAME,
    SLE_AUDIO_STREAM_RECORD,
    SLE_AUDIO_STREAM_ALERT,
    SLE_AUDIO_STREAM_VIDEO,
    SLE_AUDIO_STREAM_GUID,
};

const vector<DeviceType> g_testDeviceTypes = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX
};

class IStandardSleAudioOperationCallbackFuzzTest : public IRemoteStub<IStandardSleAudioOperationCallback> {
public:
    IStandardSleAudioOperationCallbackFuzzTest() {}
    virtual ~IStandardSleAudioOperationCallbackFuzzTest() {}

    int32_t GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override
    {
        return 0;
    }
    int32_t GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override
    {
        return 0;
    }
    int32_t IsInBandRingOpen(const std::string &device, bool &ret) override
    {
        ret = !device.empty();
        return 0;
    }
    int32_t GetSupportStreamType(const std::string &device, uint32_t &retType) override
    {
        retType = device.empty() ? SLE_AUDIO_STREAM_NONE : SLE_AUDIO_STREAM_MUSIC;
        return 0;
    }
    int32_t SetActiveSinkDevice(const std::string &device, uint32_t streamType, int32_t &ret) override
    {
        ret = device.empty() || streamType == SLE_AUDIO_STREAM_NONE ? ERROR : SUCCESS;
        return 0;
    }
    int32_t StartPlaying(const std::string &device, uint32_t streamType, int32_t timeoutMs, int32_t &ret) override
    {
        ret = device.empty() || timeoutMs < 0 ? ERROR : SUCCESS;
        return 0;
    }
    int32_t StopPlaying(const std::string &device, uint32_t streamType, int32_t &ret) override
    {
        ret = device.empty() ? ERROR : SUCCESS;
        return 0;
    }
    int32_t ConnectAllowedProfiles(const std::string &remoteAddr, int32_t &ret) override
    {
        ret = remoteAddr.empty() ? ERROR : SUCCESS;
        return 0;
    }
    int32_t SetDeviceAbsVolume(const std::string &remoteAddr, uint32_t volume, uint32_t streamType,
        int32_t &ret) override
    {
        ret = remoteAddr.empty() ? ERROR : SUCCESS;
        return 0;
    }
    int32_t SendUserSelection(const std::string &device, uint32_t streamType, int32_t eventType, int32_t &ret) override
    {
        ret = device.empty() ? ERROR : SUCCESS;
        return 0;
    }
    int32_t GetRenderPosition(const std::string &device, uint32_t &delayValue) override
    {
        delayValue = 0;
        return 0;
    }

    int32_t GetDeviceProductId(const std::string &remoteAddr, std::string &productId) override
    {
        productId = remoteAddr.empty() ? "" : "productId";
        return 0;
    }

    int32_t IsAudioLoopbackSupported(const std::string &remoteAddr, bool &ret) override
    {
        ret = !remoteAddr.empty();
        return 0;
    }
};

void EnsureCallback(SleAudioDeviceManager &manager)
{
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
}

void SleAudioDeviceManagerGetSleAudioDeviceListFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    EnsureCallback(manager);
    std::vector<AudioDeviceDescriptor> devices = {
        BuildDeviceDescriptor(fdp, PickValue(fdp, g_testDeviceTypes), "test_device")
    };
    manager.GetSleAudioDeviceList(devices);
}

void SleAudioDeviceManagerGetSleVirtualAudioDeviceListFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    EnsureCallback(manager);
    std::vector<AudioDeviceDescriptor> devices = {
        BuildDeviceDescriptor(fdp, PickValue(fdp, g_testDeviceTypes), "virtual_device")
    };
    manager.GetSleVirtualAudioDeviceList(devices);
}

void SleAudioDeviceManagerIsInBandRingOpenFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    std::string device = ConsumeString(fdp, "speaker");
    manager.IsInBandRingOpen(device);
}

void SleAudioDeviceManagerGetSupportStreamTypeFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    std::string device = ConsumeString(fdp, "speaker");
    manager.GetSupportStreamType(device);
}

void SleAudioDeviceManagerSetActiveDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    if (g_testAudioStreamUsages.size() == 0 || g_testSourceTypes.size() == 0) {
        return;
    }
    AudioDeviceDescriptor deviceDesc = BuildDeviceDescriptor(fdp,
        fdp.ConsumeBool() ? DEVICE_TYPE_NEARLINK : PickValue(fdp, g_testDeviceTypes), "nearlink_sink");
    StreamUsage streamUsage = PickValue(fdp, g_testAudioStreamUsages);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    bool isLoopback = fdp.ConsumeBool();
    manager.SetActiveDevice(deviceDesc, streamUsage, uid, isLoopback);
    AudioDeviceDescriptor deviceDescBySource = BuildDeviceDescriptor(fdp,
        fdp.ConsumeBool() ? DEVICE_TYPE_NEARLINK_IN : PickValue(fdp, g_testDeviceTypes), "nearlink_source");
    SourceType sourceType = PickValue(fdp, g_testSourceTypes);
    manager.SetActiveDevice(deviceDescBySource, sourceType, uid, isLoopback);
}

void SleAudioDeviceManagerStartPlayingFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    if (g_testAudioStreamUsages.size() == 0 || g_testSourceTypes.size() == 0) {
        return;
    }
    AudioDeviceDescriptor deviceDesc = BuildDeviceDescriptor(fdp,
        fdp.ConsumeBool() ? DEVICE_TYPE_NEARLINK : PickValue(fdp, g_testDeviceTypes), "nearlink_sink");
    StreamUsage streamUsage = PickValue(fdp, g_testAudioStreamUsages);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    bool isLoopback = fdp.ConsumeBool();
    manager.StartPlaying(deviceDesc, streamUsage, uid, isLoopback);
    AudioDeviceDescriptor deviceDescBySource = BuildDeviceDescriptor(fdp,
        fdp.ConsumeBool() ? DEVICE_TYPE_NEARLINK_IN : PickValue(fdp, g_testDeviceTypes), "nearlink_source");
    SourceType sourceType = PickValue(fdp, g_testSourceTypes);
    manager.StartPlaying(deviceDescBySource, sourceType, uid, isLoopback);
}

void SleAudioDeviceManagerStopPlayingFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    if (g_testAudioStreamUsages.size() == 0 || g_testSourceTypes.size() == 0) {
        return;
    }
    AudioDeviceDescriptor deviceDesc = BuildDeviceDescriptor(fdp,
        fdp.ConsumeBool() ? DEVICE_TYPE_NEARLINK : PickValue(fdp, g_testDeviceTypes), "nearlink_sink");
    StreamUsage streamUsage = PickValue(fdp, g_testAudioStreamUsages);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    manager.StopPlaying(deviceDesc, streamUsage, uid);
    AudioDeviceDescriptor deviceDescBySource = BuildDeviceDescriptor(fdp,
        fdp.ConsumeBool() ? DEVICE_TYPE_NEARLINK_IN : PickValue(fdp, g_testDeviceTypes), "nearlink_source");
    SourceType sourceType = PickValue(fdp, g_testSourceTypes);
    manager.StopPlaying(deviceDescBySource, sourceType, uid);
}

void SleAudioDeviceManagerConnectAllowedProfilesFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    std::string remoteAddr = ConsumeString(fdp, "macaddress");
    manager.ConnectAllowedProfiles(remoteAddr);
}

void SleAudioDeviceManagerSetDeviceAbsVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    std::string device = ConsumeString(fdp, "speaker");
    if (g_testAudioStreamTypes.size() == 0) {
        return;
    }
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    int32_t volume = fdp.ConsumeIntegral<int32_t>();
    SleVolumeConfigInfo configInfo;
    manager.deviceVolumeConfigInfo_[device] = make_pair(configInfo, configInfo);
    manager.SetDeviceAbsVolume(device, streamType, volume);
}

void SleAudioDeviceManagerSendUserSelectionFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    if (g_testAudioStreamUsages.size() == 0 || g_testSourceTypes.size() == 0) {
        return;
    }
    AudioDeviceDescriptor deviceDesc = BuildDeviceDescriptor(fdp,
        fdp.ConsumeBool() ? DEVICE_TYPE_NEARLINK : PickValue(fdp, g_testDeviceTypes), "nearlink_sink");
    StreamUsage streamUsage = PickValue(fdp, g_testAudioStreamUsages);
    manager.SendUserSelection(deviceDesc, streamUsage, fdp.ConsumeBool() ? USER_SELECT_SLE :
        static_cast<SleSelectState>(fdp.ConsumeIntegral<int32_t>()));
    AudioDeviceDescriptor deviceDescBySource = BuildDeviceDescriptor(fdp,
        fdp.ConsumeBool() ? DEVICE_TYPE_NEARLINK_IN : PickValue(fdp, g_testDeviceTypes), "nearlink_source");
    SourceType sourceType = PickValue(fdp, g_testSourceTypes);
    manager.SendUserSelection(deviceDescBySource, sourceType, fdp.ConsumeBool() ? USER_SELECT_SLE :
        static_cast<SleSelectState>(fdp.ConsumeIntegral<int32_t>()));
}

void SleAudioDeviceManagerGetStreamUsagesBySleStreamTypeFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }

    if (g_testSleAudioStreamTypes.size() == 0) {
        return;
    }
    uint32_t streamType = static_cast<uint32_t>(PickValue(fdp, g_testSleAudioStreamTypes));
    manager.GetStreamUsagesBySleStreamType(streamType);
}

void SleAudioDeviceManagerGetSourceTypesBySleStreamTypeFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }

    if (g_testSleAudioStreamTypes.size() == 0) {
        return;
    }
    uint32_t streamType = static_cast<uint32_t>(PickValue(fdp, g_testSleAudioStreamTypes));
    manager.GetSourceTypesBySleStreamType(streamType);
}

void SleAudioDeviceManagerAddNearlinkDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }

    AudioDeviceDescriptor deviceDesc;
    if (g_testDeviceTypes.size() == 0) {
        return;
    }
    deviceDesc.deviceType_ = PickValue(fdp, g_testDeviceTypes);
    manager.AddNearlinkDevice(deviceDesc);
    manager.RemoveNearlinkDevice(deviceDesc);
}

void SleAudioDeviceManagerUpdateSleStreamTypeCountFuzzTest(FuzzedDataProvider& fdp)
{
    static const vector<AudioStreamStatus> testAudioStreamStatuses = {
        STREAM_STATUS_NEW,
        STREAM_STATUS_STARTED,
        STREAM_STATUS_PAUSED,
        STREAM_STATUS_STOPPED,
        STREAM_STATUS_RELEASED,
    };
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    std::shared_ptr<AudioStreamDescriptor> streamDesc = make_shared<AudioStreamDescriptor>();
    bool isRemoved = fdp.ConsumeBool();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = make_shared<AudioDeviceDescriptor>();
    if (g_testDeviceTypes.size() == 0 || testAudioStreamStatuses.size() == 0) {
        return;
    }
    bool isDeviceType = fdp.ConsumeBool();
    if (isDeviceType) {
        deviceDesc->deviceType_ = DEVICE_TYPE_NEARLINK;
        streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    } else {
        deviceDesc->deviceType_ = PickValue(fdp, g_testDeviceTypes);
        streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    }
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->oldDeviceDescs_.push_back(deviceDesc);
    streamDesc->streamStatus_ = PickValue(fdp, testAudioStreamStatuses);

    manager.UpdateSleStreamTypeCount(streamDesc, isRemoved);
}

void SleAudioDeviceManagerSetNearlinkDeviceMuteFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }

    if (g_testAudioStreamTypes.size() == 0) {
        return;
    }
    SleVolumeConfigInfo configInfo;
    std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo> pairConfigInfo = make_pair(configInfo, configInfo);
    std::string device = ConsumeString(fdp, "test_device");
    manager.deviceVolumeConfigInfo_.insert(make_pair(device, pairConfigInfo));
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    bool isMute = fdp.ConsumeBool();
    manager.SetNearlinkDeviceMute(device, streamType, isMute);
}

void SleAudioDeviceManagerSetNearlinkDeviceVolumeLevelFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }

    SleVolumeConfigInfo configInfo;
    std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo> pairConfigInfo = make_pair(configInfo, configInfo);
    std::string device = ConsumeString(fdp, "test_device");
    manager.deviceVolumeConfigInfo_.insert(make_pair(device, pairConfigInfo));
    if (g_testAudioStreamTypes.size() == 0) {
        return;
    }
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    int32_t volumeLevel = fdp.ConsumeIntegral<int32_t>();
    int32_t volumeDegree = fdp.ConsumeIntegral<int32_t>();
    VolumeScale volume{volumeLevel, volumeDegree};
    manager.SetNearlinkDeviceVolume(device, streamType, volume);
}

void SleAudioDeviceManagerGetVolumeLevelByVolumeTypeFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }

    SleVolumeConfigInfo configInfo;
    std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo> pairConfigInfo = make_pair(configInfo, configInfo);
    std::string device = ConsumeString(fdp, "test_mac_address");
    manager.deviceVolumeConfigInfo_.insert(make_pair(device, pairConfigInfo));
    if (g_testAudioStreamTypes.size() == 0) {
        return;
    }
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(PickValue(fdp, g_testAudioStreamTypes));
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.macAddress_ = device;
    manager.GetVolumeLevelByVolumeType(volumeType, deviceDesc);
}

void SleAudioDeviceManagerSetSleAudioOperationCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (fdp.ConsumeBool()) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    } else {
        manager.SetSleAudioOperationCallback(nullptr);
    }
}

void SleAudioDeviceManagerSetActiveSinkDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    EnsureCallback(manager);
    std::string device = ConsumeString(fdp, "nearlink_sink");
    uint32_t streamType = fdp.ConsumeBool() ? static_cast<uint32_t>(PickValue(fdp, g_testSleAudioStreamTypes)) :
        fdp.ConsumeIntegral<uint32_t>();
    manager.SetActiveSinkDevice(device, streamType);
}

void SleAudioDeviceManagerStartPlayingByDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    EnsureCallback(manager);
    std::string device = ConsumeString(fdp, "nearlink_sink");
    uint32_t streamType = fdp.ConsumeBool() ? static_cast<uint32_t>(PickValue(fdp, g_testSleAudioStreamTypes)) :
        fdp.ConsumeIntegral<uint32_t>();
    int32_t timeoutMs = fdp.ConsumeIntegralInRange<int32_t>(-1, SLE_EXTENDED_TIMEOUT_MS);
    manager.StartPlaying(device, streamType, timeoutMs);
}

void SleAudioDeviceManagerStopPlayingByDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    EnsureCallback(manager);
    std::string device = ConsumeString(fdp, "nearlink_sink");
    uint32_t streamType = fdp.ConsumeBool() ? static_cast<uint32_t>(PickValue(fdp, g_testSleAudioStreamTypes)) :
        fdp.ConsumeIntegral<uint32_t>();
    manager.StopPlaying(device, streamType);
}

void SleAudioDeviceManagerSetDeviceAbsVolumeBySleStreamFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    EnsureCallback(manager);
    std::string device = ConsumeString(fdp, "nearlink_sink");
    uint32_t volume = fdp.ConsumeIntegral<uint32_t>();
    uint32_t streamType = fdp.ConsumeBool() ? static_cast<uint32_t>(PickValue(fdp, g_testSleAudioStreamTypes)) :
        fdp.ConsumeIntegral<uint32_t>();
    manager.SetDeviceAbsVolume(device, volume, streamType);
}

void SleAudioDeviceManagerSendUserSelectionBySleStreamFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    EnsureCallback(manager);
    std::string device = ConsumeString(fdp, "nearlink_sink");
    uint32_t streamType = fdp.ConsumeBool() ? static_cast<uint32_t>(PickValue(fdp, g_testSleAudioStreamTypes)) :
        fdp.ConsumeIntegral<uint32_t>();
    int32_t eventType = fdp.ConsumeIntegral<int32_t>();
    manager.SendUserSelection(device, streamType, eventType);
}

void SleAudioDeviceManagerGetRenderPositionFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    EnsureCallback(manager);
    std::string device = ConsumeString(fdp, "nearlink_sink");
    uint32_t delayValue = fdp.ConsumeIntegral<uint32_t>();
    manager.GetRenderPosition(device, delayValue);
}

void SleAudioDeviceManagerIsAudioLoopbackSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    EnsureCallback(manager);
    std::string remoteAddr = ConsumeString(fdp, "nearlink_sink");
    manager.IsAudioLoopbackSupported(remoteAddr);
}

void SleAudioDeviceManagerGetSleStreamTypeByStreamUsageFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    StreamUsage streamUsage = PickValue(fdp, g_testAudioStreamUsages);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    bool isLoopback = fdp.ConsumeBool();
    manager.GetSleStreamTypeByStreamUsage(streamUsage, uid, isLoopback);
}

void SleAudioDeviceManagerGetSleStreamTypeBySourceTypeFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    SourceType sourceType = PickValue(fdp, g_testSourceTypes);
    manager.GetSleStreamTypeBySourceType(sourceType);
}

void SleAudioDeviceManagerAddRemoveNearlinkDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    AudioDeviceDescriptor deviceDesc = BuildDeviceDescriptor(fdp,
        fdp.ConsumeBool() ? DEVICE_TYPE_NEARLINK : PickValue(fdp, g_testDeviceTypes), "nearlink_device");
    if (fdp.ConsumeBool()) {
        deviceDesc.connectState_ = CONNECTED;
    }
    manager.AddNearlinkDevice(deviceDesc);
    manager.RemoveNearlinkDevice(deviceDesc);
}

void SleAudioDeviceManagerGetVolumeDegreeByVolumeTypeFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    std::string device = ConsumeString(fdp, "test_mac_address");
    SleVolumeConfigInfo musicInfo(STREAM_MUSIC, fdp.ConsumeIntegral<int32_t>());
    musicInfo.volumeDegree = fdp.ConsumeIntegral<int32_t>();
    musicInfo.isMute = fdp.ConsumeBool();
    SleVolumeConfigInfo callInfo(STREAM_VOICE_CALL, fdp.ConsumeIntegral<int32_t>());
    callInfo.volumeDegree = fdp.ConsumeIntegral<int32_t>();
    manager.deviceVolumeConfigInfo_[device] = make_pair(musicInfo, callInfo);
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.macAddress_ = device;
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(PickValue(fdp, g_testAudioStreamTypes));
    manager.GetVolumeDegreeByVolumeType(volumeType, deviceDesc);
}

void SleAudioDeviceManagerGetMuteStatusByVolumeTypeFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    std::string device = ConsumeString(fdp, "test_mac_address");
    SleVolumeConfigInfo musicInfo(STREAM_MUSIC, fdp.ConsumeIntegral<int32_t>());
    musicInfo.isMute = fdp.ConsumeBool();
    SleVolumeConfigInfo callInfo(STREAM_VOICE_CALL, fdp.ConsumeIntegral<int32_t>());
    manager.deviceVolumeConfigInfo_[device] = make_pair(musicInfo, callInfo);
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(PickValue(fdp, g_testAudioStreamTypes));
    manager.GetMuteStatusByVolumeType(volumeType, device);
}

void SleAudioDeviceManagerGetActivatedDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    std::string deviceAddr = ConsumeString(fdp, "test_device");
    uint32_t streamType = fdp.ConsumeBool() ? static_cast<uint32_t>(PickValue(fdp, g_testSleAudioStreamTypes)) :
        fdp.ConsumeIntegral<uint32_t>();
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    manager.UpdateStreamTypeMap(deviceAddr, streamType, sessionId, true);
    manager.startedSleStreamType_[deviceAddr][streamType].isStarted = fdp.ConsumeBool();
    manager.GetActivatedDevice();
}

void SleAudioDeviceManagerIsGameAppFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    manager.IsGameApp(uid);
}

void SleAudioDeviceManagerUpdateStreamTypeMapFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    std::string deviceAddr = ConsumeString(fdp, "test_device");
    uint32_t streamType = fdp.ConsumeIntegral<uint32_t>();
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    bool isAdd = fdp.ConsumeBool();
    manager.UpdateStreamTypeMap(deviceAddr, streamType, sessionId, isAdd);
}

void SleAudioDeviceManagerResetSleStreamTypeCountFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    std::string deviceAddr = ConsumeString(fdp, "test_device");
    uint32_t streamType = fdp.ConsumeBool() ? static_cast<uint32_t>(PickValue(fdp, g_testSleAudioStreamTypes)) :
        fdp.ConsumeIntegral<uint32_t>();
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    if (fdp.ConsumeBool()) {
        manager.UpdateStreamTypeMap(deviceAddr, streamType, sessionId, true);
    }
    manager.ResetSleStreamTypeCount(deviceAddr);
}

void SleAudioDeviceManagerGetNearlinkStreamTypeMapByDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    std::string deviceAddr = ConsumeString(fdp, "test_device");
    uint32_t streamType = fdp.ConsumeBool() ? static_cast<uint32_t>(PickValue(fdp, g_testSleAudioStreamTypes)) :
        fdp.ConsumeIntegral<uint32_t>();
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    if (fdp.ConsumeBool()) {
        manager.UpdateStreamTypeMap(deviceAddr, streamType, sessionId, true);
    }
    manager.GetNearlinkStreamTypeMapByDevice(deviceAddr);
}

void SleAudioDeviceManagerIsNearlinkDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (g_testDeviceTypes.size() == 0) {
        return;
    }
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    manager.IsNearlinkDevice(deviceType);
}

void SleAudioDeviceManagerIsMoveToNearlinkDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    auto streamDesc = make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_[0]->deviceType_ = PickValue(fdp, g_testDeviceTypes);
    manager.IsMoveToNearlinkDevice(streamDesc);
}

void SleAudioDeviceManagerIsNearlinkMoveToOtherDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    auto streamDesc = make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(make_shared<AudioDeviceDescriptor>());
    streamDesc->oldDeviceDescs_.push_back(make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_[0]->deviceType_ = PickValue(fdp, g_testDeviceTypes);
    streamDesc->oldDeviceDescs_[0]->deviceType_ = PickValue(fdp, g_testDeviceTypes);
    manager.IsNearlinkMoveToOtherDevice(streamDesc);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    SleAudioDeviceManagerSetSleAudioOperationCallbackFuzzTest,
    SleAudioDeviceManagerGetSleAudioDeviceListFuzzTest,
    SleAudioDeviceManagerGetSleVirtualAudioDeviceListFuzzTest,
    SleAudioDeviceManagerIsInBandRingOpenFuzzTest,
    SleAudioDeviceManagerGetSupportStreamTypeFuzzTest,
    SleAudioDeviceManagerSetActiveSinkDeviceFuzzTest,
    SleAudioDeviceManagerSetActiveDeviceFuzzTest,
    SleAudioDeviceManagerStartPlayingByDeviceFuzzTest,
    SleAudioDeviceManagerStartPlayingFuzzTest,
    SleAudioDeviceManagerStopPlayingByDeviceFuzzTest,
    SleAudioDeviceManagerStopPlayingFuzzTest,
    SleAudioDeviceManagerConnectAllowedProfilesFuzzTest,
    SleAudioDeviceManagerSetDeviceAbsVolumeBySleStreamFuzzTest,
    SleAudioDeviceManagerSetDeviceAbsVolumeFuzzTest,
    SleAudioDeviceManagerSendUserSelectionBySleStreamFuzzTest,
    SleAudioDeviceManagerSendUserSelectionFuzzTest,
    SleAudioDeviceManagerGetRenderPositionFuzzTest,
    SleAudioDeviceManagerIsAudioLoopbackSupportedFuzzTest,
    SleAudioDeviceManagerGetSleStreamTypeByStreamUsageFuzzTest,
    SleAudioDeviceManagerGetSleStreamTypeBySourceTypeFuzzTest,
    SleAudioDeviceManagerGetStreamUsagesBySleStreamTypeFuzzTest,
    SleAudioDeviceManagerGetSourceTypesBySleStreamTypeFuzzTest,
    SleAudioDeviceManagerAddNearlinkDeviceFuzzTest,
    SleAudioDeviceManagerAddRemoveNearlinkDeviceFuzzTest,
    SleAudioDeviceManagerUpdateSleStreamTypeCountFuzzTest,
    SleAudioDeviceManagerSetNearlinkDeviceMuteFuzzTest,
    SleAudioDeviceManagerSetNearlinkDeviceVolumeLevelFuzzTest,
    SleAudioDeviceManagerGetVolumeLevelByVolumeTypeFuzzTest,
    SleAudioDeviceManagerGetVolumeDegreeByVolumeTypeFuzzTest,
    SleAudioDeviceManagerGetMuteStatusByVolumeTypeFuzzTest,
    SleAudioDeviceManagerGetActivatedDeviceFuzzTest,
    SleAudioDeviceManagerIsGameAppFuzzTest,
    SleAudioDeviceManagerUpdateStreamTypeMapFuzzTest,
    SleAudioDeviceManagerResetSleStreamTypeCountFuzzTest,
    SleAudioDeviceManagerGetNearlinkStreamTypeMapByDeviceFuzzTest,
    SleAudioDeviceManagerIsNearlinkDeviceFuzzTest,
    SleAudioDeviceManagerIsMoveToNearlinkDeviceFuzzTest,
    SleAudioDeviceManagerIsNearlinkMoveToOtherDeviceFuzzTest,
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
extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    OHOS::AudioStandard::Init();
    return 0;
}
