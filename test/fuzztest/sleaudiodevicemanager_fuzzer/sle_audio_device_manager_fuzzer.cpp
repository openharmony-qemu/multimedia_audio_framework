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

#include "audio_log.h"
#include "sle_audio_device_manager.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)();

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

    void GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override {}
    void GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override {}
    bool IsInBandRingOpen(const std::string &device) override
    {
        return false;
    }
    uint32_t GetSupportStreamType(const std::string &device) override
    {
        return 0;
    }
    int32_t SetActiveSinkDevice(const std::string &device, uint32_t streamType) override
    {
        return 0;
    }
    int32_t StartPlaying(const std::string &device, uint32_t streamType) override
    {
        return 0;
    }
    int32_t StopPlaying(const std::string &device, uint32_t streamType) override
    {
        return 0;
    }
    int32_t ConnectAllowedProfiles(const std::string &remoteAddr) override
    {
        return 0;
    }
    int32_t SetDeviceAbsVolume(const std::string &remoteAddr, uint32_t volume, uint32_t streamType) override
    {
        return 0;
    }
    int32_t SendUserSelection(const std::string &device, uint32_t streamType) override
    {
        return 0;
    }

    int32_t GetRenderPosition(const std::string &device, uint32_t &delayValue) override
    {
        delayValue = 0;
        return 0;
    }
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
    if (g_dataSize < g_pos) {
        return object;
    }
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

void SleAudioDeviceManagerGetSleAudioDeviceListFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    std::vector<AudioDeviceDescriptor> devices;
    manager.GetSleAudioDeviceList(devices);
}

void SleAudioDeviceManagerGetSleVirtualAudioDeviceListFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    std::vector<AudioDeviceDescriptor> devices;
    manager.GetSleVirtualAudioDeviceList(devices);
}

void SleAudioDeviceManagerIsInBandRingOpenFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    std::string device;
    manager.IsInBandRingOpen(device);
}

void SleAudioDeviceManagerGetSupportStreamTypeFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    std::string device;
    manager.GetSupportStreamType(device);
}

void SleAudioDeviceManagerSetActiveDeviceFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    AudioDeviceDescriptor deviceDesc;
    if (g_testAudioStreamUsages.size() == 0 || g_testSourceTypes.size() == 0) {
        return;
    }
    StreamUsage streamUsage = g_testAudioStreamUsages[GetData<uint32_t>() % g_testAudioStreamUsages.size()];
    manager.SetActiveDevice(deviceDesc, streamUsage);
    AudioDeviceDescriptor deviceDescBySource;
    SourceType sourceType = g_testSourceTypes[GetData<uint32_t>() % g_testSourceTypes.size()];
    manager.SetActiveDevice(deviceDescBySource, sourceType);
}

void SleAudioDeviceManagerStartPlayingFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    AudioDeviceDescriptor deviceDesc;
    if (g_testAudioStreamUsages.size() == 0 || g_testSourceTypes.size() == 0) {
        return;
    }
    StreamUsage streamUsage = g_testAudioStreamUsages[GetData<uint32_t>() % g_testAudioStreamUsages.size()];
    manager.StartPlaying(deviceDesc, streamUsage);
    AudioDeviceDescriptor deviceDescBySource;
    SourceType sourceType = g_testSourceTypes[GetData<uint32_t>() % g_testSourceTypes.size()];
    manager.StartPlaying(deviceDescBySource, sourceType);
}

void SleAudioDeviceManagerStopPlayingFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    AudioDeviceDescriptor deviceDesc;
    if (g_testAudioStreamUsages.size() == 0 || g_testSourceTypes.size() == 0) {
        return;
    }
    StreamUsage streamUsage = g_testAudioStreamUsages[GetData<uint32_t>() % g_testAudioStreamUsages.size()];
    manager.StopPlaying(deviceDesc, streamUsage);
    AudioDeviceDescriptor deviceDescBySource;
    SourceType sourceType = g_testSourceTypes[GetData<uint32_t>() % g_testSourceTypes.size()];
    manager.StopPlaying(deviceDescBySource, sourceType);
}

void SleAudioDeviceManagerConnectAllowedProfilesFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    std::string remoteAddr;
    manager.ConnectAllowedProfiles(remoteAddr);
}

void SleAudioDeviceManagerSetDeviceAbsVolumeFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    std::string device;
    if (g_testAudioStreamTypes.size() == 0) {
        return;
    }
    AudioStreamType streamType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    int32_t volume = GetData<int32_t>();
    manager.SetDeviceAbsVolume(device, streamType, volume);
}

void SleAudioDeviceManagerSendUserSelectionFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }
    AudioDeviceDescriptor deviceDesc;
    if (g_testAudioStreamUsages.size() == 0 || g_testSourceTypes.size() == 0) {
        return;
    }
    StreamUsage streamUsage = g_testAudioStreamUsages[GetData<uint32_t>() % g_testAudioStreamUsages.size()];
    manager.SendUserSelection(deviceDesc, streamUsage);
    AudioDeviceDescriptor deviceDescBySource;
    SourceType sourceType = g_testSourceTypes[GetData<uint32_t>() % g_testSourceTypes.size()];
    manager.SendUserSelection(deviceDescBySource, sourceType);
}

void SleAudioDeviceManagerGetStreamUsagesBySleStreamTypeFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }

    if (g_testSleAudioStreamTypes.size() == 0) {
        return;
    }
    uint32_t streamType = static_cast<uint32_t>(
        g_testSleAudioStreamTypes[GetData<uint32_t>() % g_testSleAudioStreamTypes.size()]);
    manager.GetStreamUsagesBySleStreamType(streamType);
}

void SleAudioDeviceManagerGetSourceTypesBySleStreamTypeFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }

    if (g_testSleAudioStreamTypes.size() == 0) {
        return;
    }
    uint32_t streamType = static_cast<uint32_t>(
        g_testSleAudioStreamTypes[GetData<uint32_t>() % g_testSleAudioStreamTypes.size()]);
    manager.GetSourceTypesBySleStreamType(streamType);
}

void SleAudioDeviceManagerAddNearlinkDeviceFuzzTest()
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
    deviceDesc.deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    manager.AddNearlinkDevice(deviceDesc);
    manager.RemoveNearlinkDevice(deviceDesc);
}

void SleAudioDeviceManagerUpdateSleStreamTypeCountFuzzTest()
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
    bool isRemoved = GetData<bool>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = make_shared<AudioDeviceDescriptor>();
    if (g_testDeviceTypes.size() == 0 || testAudioStreamStatuses.size() == 0) {
        return;
    }
    bool isDeviceType = GetData<bool>();
    if (isDeviceType) {
        deviceDesc->deviceType_ = DEVICE_TYPE_NEARLINK;
        streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    } else {
        deviceDesc->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
        streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    }
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->oldDeviceDescs_.push_back(deviceDesc);
    streamDesc->streamStatus_ = testAudioStreamStatuses[GetData<uint32_t>() % testAudioStreamStatuses.size()];

    manager.UpdateSleStreamTypeCount(streamDesc, isRemoved);
}

void SleAudioDeviceManagerSetNearlinkDeviceMuteFuzzTest()
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
    std::string device = "test_device";
    manager.deviceVolumeConfigInfo_.insert(make_pair(device, pairConfigInfo));
    AudioStreamType streamType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    bool isMute = GetData<bool>();
    manager.SetNearlinkDeviceMute(device, streamType, isMute);
}

void SleAudioDeviceManagerSetNearlinkDeviceVolumeLevelFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }

    SleVolumeConfigInfo configInfo;
    std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo> pairConfigInfo = make_pair(configInfo, configInfo);
    std::string device = "test_device";
    manager.deviceVolumeConfigInfo_.insert(make_pair(device, pairConfigInfo));
    if (g_testAudioStreamTypes.size() == 0) {
        return;
    }
    AudioStreamType streamType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    int32_t volumeLevel = GetData<int32_t>();
    manager.SetNearlinkDeviceVolumeLevel(device, streamType, volumeLevel);
}

void SleAudioDeviceManagerGetVolumeLevelByVolumeTypeFuzzTest()
{
    SleAudioDeviceManager &manager = SleAudioDeviceManager::GetInstance();
    if (manager.callback_ == nullptr) {
        sptr<IStandardSleAudioOperationCallback> callback = new IStandardSleAudioOperationCallbackFuzzTest();
        manager.SetSleAudioOperationCallback(callback);
    }

    SleVolumeConfigInfo configInfo;
    std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo> pairConfigInfo = make_pair(configInfo, configInfo);
    std::string device = "test_mac_address";
    manager.deviceVolumeConfigInfo_.insert(make_pair(device, pairConfigInfo));
    if (g_testAudioStreamTypes.size() == 0) {
        return;
    }
    AudioVolumeType volumeType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.macAddress_ = "test_mac_address";
    manager.GetVolumeLevelByVolumeType(volumeType, deviceDesc);
}

TestPtr g_testPtrs[] = {
    SleAudioDeviceManagerGetSleAudioDeviceListFuzzTest,
    SleAudioDeviceManagerGetSleVirtualAudioDeviceListFuzzTest,
    SleAudioDeviceManagerIsInBandRingOpenFuzzTest,
    SleAudioDeviceManagerGetSupportStreamTypeFuzzTest,
    SleAudioDeviceManagerSetActiveDeviceFuzzTest,
    SleAudioDeviceManagerStartPlayingFuzzTest,
    SleAudioDeviceManagerStopPlayingFuzzTest,
    SleAudioDeviceManagerConnectAllowedProfilesFuzzTest,
    SleAudioDeviceManagerSetDeviceAbsVolumeFuzzTest,
    SleAudioDeviceManagerSendUserSelectionFuzzTest,
    SleAudioDeviceManagerGetStreamUsagesBySleStreamTypeFuzzTest,
    SleAudioDeviceManagerGetSourceTypesBySleStreamTypeFuzzTest,
    SleAudioDeviceManagerAddNearlinkDeviceFuzzTest,
    SleAudioDeviceManagerUpdateSleStreamTypeCountFuzzTest,
    SleAudioDeviceManagerSetNearlinkDeviceMuteFuzzTest,
    SleAudioDeviceManagerSetNearlinkDeviceVolumeLevelFuzzTest,
    SleAudioDeviceManagerGetVolumeLevelByVolumeTypeFuzzTest,
};

void FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
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
    return;
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