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
#include "audio_zone.h"
#include "audio_zone_client_manager.h"
#include "audio_zone_interrupt_reporter.h"
#include "audio_zone_service.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)();

class IStandardAudioZoneClientFuzzTest : public IStandardAudioZoneClient {
public:
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    ErrCode OnAudioZoneAdd(const AudioZoneDescriptor &zoneDescriptor) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_ADD_EVENT;
        Notify();
        return 0;
    }

    ErrCode OnAudioZoneRemove(int32_t zoneId) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_REMOVE_EVENT;
        recvEvent_.zoneId = zoneId;
        Notify();
        return 0;
    }

    ErrCode OnAudioZoneChange(int32_t zoneId, const AudioZoneDescriptor& zoneDescriptor,
        int32_t reason) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_CHANGE_EVENT;
        recvEvent_.zoneId = zoneId;
        Notify();
        return 0;
    }

    ErrCode OnInterruptEvent(int32_t zoneId,
        const std::vector<std::map<AudioInterrupt, int32_t>>& ipcInterrupts,
        int32_t reason) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT;
        recvEvent_.zoneId = zoneId;
        Notify();
        return 0;
    }

    ErrCode OnInterruptEvent(int32_t zoneId, const std::string& deviceTag,
        const std::vector<std::map<AudioInterrupt, int32_t>>& ipcInterrupts,
        int32_t reason) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT;
        recvEvent_.zoneId = zoneId;
        recvEvent_.deviceTag = deviceTag;
        Notify();
        return 0;
    }

    ErrCode SetSystemVolume(int32_t zoneId, int32_t volumeType, int32_t volumeLevel, int32_t volumeFlag) override
    {
        volumeLevel_ = volumeLevel;
        Notify();
        return 0;
    }

    ErrCode GetSystemVolume(int32_t zoneId, int32_t volumeType, float& outVolume) override
    {
        Notify();
        return volumeLevel_;
    }

    void Notify()
    {
        std::unique_lock<std::mutex> lock(waitLock_);
        waitStatus_ = 1;
        waiter_.notify_one();
    }

    void Wait()
    {
        std::unique_lock<std::mutex> lock(waitLock_);
        if (waitStatus_ == 0) {
            waiter_.wait(lock, [this] {
                return waitStatus_ != 0;
            });
        }
        waitStatus_ = 0;
    }

    struct AudioZoneEvent recvEvent_;
    std::condition_variable waiter_;
    std::mutex waitLock_;
    int32_t waitStatus_ = 0;
    int32_t volumeLevel_ = 0;
};

using AudioZoneFocusList = std::list<std::pair<AudioInterrupt, AudioFocuState>>;

const vector<StreamUsage> g_testStreamUsages = {
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

const vector<RouterType> g_testRouterTypes = {
    ROUTER_TYPE_NONE,
    ROUTER_TYPE_DEFAULT,
    ROUTER_TYPE_STREAM_FILTER,
    ROUTER_TYPE_PACKAGE_FILTER,
    ROUTER_TYPE_COCKPIT_PHONE,
    ROUTER_TYPE_PRIVACY_PRIORITY,
    ROUTER_TYPE_PUBLIC_PRIORITY,
    ROUTER_TYPE_PAIR_DEVICE,
    ROUTER_TYPE_USER_SELECT,
    ROUTER_TYPE_APP_SELECT,
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

void AudioZoneServiceCreateAudioZoneFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    std::string name = "testZone";
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> manager = std::make_shared<AudioZoneClientManager>(nullptr);
    int32_t zoneId = GetData<int32_t>();
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(manager, name, context);
    audioZoneService.zoneMaps_.insert(make_pair(zoneId, zone));
    audioZoneService.CreateAudioZone(name, context);
}

void AudioZoneServiceReleaseAudioZoneFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    std::string name = "testZone";
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> manager = std::make_shared<AudioZoneClientManager>(nullptr);
    int32_t zoneId = GetData<int32_t>();
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(manager, name, context);
    audioZoneService.zoneMaps_.insert(make_pair(zoneId, zone));
    audioZoneService.ReleaseAudioZone(zoneId);
}

void AudioZoneServiceGetAllAudioZoneFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    std::string name = "testZone";
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> manager = std::make_shared<AudioZoneClientManager>(nullptr);
    int32_t zoneId = GetData<int32_t>();
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(manager, name, context);
    audioZoneService.zoneMaps_.insert(make_pair(zoneId, zone));
    audioZoneService.GetAllAudioZone();
}

void AudioZoneServiceBindDeviceToAudioZoneFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    AudioZoneContext context;
    std::string name = "testZone";
    std::shared_ptr<AudioZoneClientManager> manager = std::make_shared<AudioZoneClientManager>(nullptr);
    int32_t zoneId = GetData<int32_t>();
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(manager, name, context);
    audioZoneService.zoneMaps_.insert(make_pair(zoneId, zone));
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    devices.push_back(audioDeviceDescriptor);
    audioZoneService.BindDeviceToAudioZone(zoneId, devices);
}

void AudioZoneServiceRemoveDeviceFromGlobalFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = make_shared<AudioDeviceDescriptor>();
    audioZoneService.RemoveDeviceFromGlobal(audioDeviceDescriptor);
}

void AudioZoneServiceUnBindDeviceToAudioZoneFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    AudioZoneContext context;
    std::string name = "testZone";
    std::shared_ptr<AudioZoneClientManager> manager = std::make_shared<AudioZoneClientManager>(nullptr);
    int32_t zoneId = GetData<int32_t>();
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(manager, name, context);
    audioZoneService.zoneMaps_.insert(make_pair(zoneId, zone));
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    devices.push_back(audioDeviceDescriptor);
    audioZoneService.UnBindDeviceToAudioZone(zoneId, devices);
}

void AudioZoneServiceRegisterAudioZoneClientFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    int32_t clientPid = GetData<int32_t>();
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientFuzzTest();
    audioZoneService.RegisterAudioZoneClient(clientPid, client);
}

void AudioZoneServiceUnRegisterAudioZoneClientFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    AudioZoneContext context;
    std::string name = "testZone";
    std::shared_ptr<AudioZoneClientManager> manager = std::make_shared<AudioZoneClientManager>(nullptr);
    int32_t zoneId = GetData<int32_t>();
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(manager, name, context);
    audioZoneService.zoneMaps_.insert(make_pair(zoneId, zone));
    int32_t clientPid = GetData<int32_t>();
    audioZoneService.UnRegisterAudioZoneClient(clientPid);
}

void AudioZoneServiceInjectInterruptToAudioZoneFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    AudioZoneContext context;
    std::string name = "testZone";
    std::shared_ptr<AudioZoneClientManager> manager = std::make_shared<AudioZoneClientManager>(nullptr);
    int32_t zoneId = GetData<int32_t>();
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(manager, name, context);
    audioZoneService.zoneMaps_.insert(make_pair(zoneId, zone));
    AudioZoneFocusList interrupts = audioZoneService.GetAudioInterruptForZone(zoneId);
    audioZoneService.InjectInterruptToAudioZone(zoneId, interrupts);
}

void AudioZoneServiceFetchOutputDevicesFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    AudioZoneContext context;
    std::string name = "testZone";
    std::shared_ptr<AudioZoneClientManager> manager = std::make_shared<AudioZoneClientManager>(nullptr);
    int32_t zoneId = GetData<int32_t>();
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(manager, name, context);
    audioZoneService.zoneMaps_.insert(make_pair(zoneId, zone));
    if (g_testStreamUsages.size() == 0 || g_testRouterTypes.size() == 0) {
        return;
    }
    StreamUsage streamUsage = g_testStreamUsages[GetData<uint32_t>() % g_testStreamUsages.size()];
    int32_t clientUid = GetData<int32_t>();
    RouterType bypassType = g_testRouterTypes[GetData<uint32_t>() % g_testRouterTypes.size()];
    audioZoneService.FetchOutputDevices(zoneId, streamUsage, clientUid, bypassType);
}

void AudioZoneServiceFetchInputDeviceFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    AudioZoneContext context;
    std::string name = "testZone";
    std::shared_ptr<AudioZoneClientManager> manager = std::make_shared<AudioZoneClientManager>(nullptr);
    int32_t zoneId = GetData<int32_t>();
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(manager, name, context);
    audioZoneService.zoneMaps_.insert(make_pair(zoneId, zone));
    if (g_testSourceTypes.size() == 0) {
        return;
    }
    SourceType sourceType = g_testSourceTypes[GetData<uint32_t>() % g_testSourceTypes.size()];
    int32_t clientUid = GetData<int32_t>();
    audioZoneService.FetchInputDevice(zoneId, sourceType, clientUid);
}

void AudioZoneServiceGetZoneStringDescriptorFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    AudioZoneContext context;
    std::string name = "testZone";
    std::shared_ptr<AudioZoneClientManager> manager = std::make_shared<AudioZoneClientManager>(nullptr);
    int32_t zoneId = GetData<int32_t>();
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(manager, name, context);
    audioZoneService.zoneMaps_.insert(make_pair(zoneId, zone));
    audioZoneService.GetZoneStringDescriptor(zoneId);
}

void AudioZoneServiceUpdateDeviceFromGlobalForAllZoneFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    AudioZoneContext context;
    std::string name = "testZone";
    std::shared_ptr<AudioZoneClientManager> manager = std::make_shared<AudioZoneClientManager>(nullptr);
    int32_t zoneId = GetData<int32_t>();
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(manager, name, context);
    audioZoneService.zoneMaps_.insert(make_pair(zoneId, zone));
    std::shared_ptr<AudioDeviceDescriptor> device = make_shared<AudioDeviceDescriptor>();
    audioZoneService.UpdateDeviceFromGlobalForAllZone(device);
}

void AudioZoneServiceClearAudioFocusBySessionIDFuzzTest()
{
    AudioZoneService &audioZoneService = AudioZoneService::GetInstance();

    audioZoneService.DeInit();
    audioZoneService.Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    int32_t sessionID = GetData<int32_t>();
    audioZoneService.ClearAudioFocusBySessionID(sessionID);
}

void AudioZoneClientManagerRegisterAudioZoneClientFuzzTest()
{
    AudioZoneClientManager &audioZoneClientManager = AudioZoneClientManager::GetInstance();
    pid_t clientPid = GetData<pid_t>();
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientFuzzTest();
    audioZoneClientManager.RegisterAudioZoneClient(clientPid, client);
    audioZoneClientManager.clients_.clear();
}

void AudioZoneClientManagerUnRegisterAudioZoneClientFuzzTest()
{
    AudioZoneClientManager &audioZoneClientManager = AudioZoneClientManager::GetInstance();
    pid_t clientPid = GetData<pid_t>();
    audioZoneClientManager.UnRegisterAudioZoneClient(clientPid);
}

void AudioZoneClientManagerDispatchEventFuzzTest()
{
    static const vector<AudioZoneEventType> testAudioZoneEventTypes = {
        AUDIO_ZONE_INVALID_EVENT,
        AUDIO_ZONE_ADD_EVENT,
        AUDIO_ZONE_REMOVE_EVENT,
        AUDIO_ZONE_CHANGE_EVENT,
        AUDIO_ZONE_INTERRUPT_EVENT,
    };
    if (testAudioZoneEventTypes.size() == 0) {
        return;
    }
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientFuzzTest();
    AudioZoneClientManager &audioZoneClientManager = AudioZoneClientManager::GetInstance();
    std::shared_ptr<AudioZoneEvent> event = std::make_shared<AudioZoneEvent>();
    event->type = testAudioZoneEventTypes[GetData<uint32_t>() % testAudioZoneEventTypes.size()];
    event->clientPid = GetData<pid_t>();
    event->zoneId = GetData<int32_t>();
    event->deviceTag = "testDeviceTag";
    event->descriptor = std::make_shared<AudioZoneDescriptor>();
    event->zoneChangeReason = AudioZoneChangeReason::BIND_NEW_DEVICE;
    AudioInterrupt interrupt;
    AudioFocuState focusState = ACTIVE;
    event->interrupts.push_back(make_pair(interrupt, focusState));
    event->zoneInterruptReason = AudioZoneInterruptReason::UNBIND_APP_FROM_ZONE;
    bool isNull = GetData<bool>();
    if (isNull) {
        event->deviceTag.clear();
        event->descriptor = nullptr;
    }
    audioZoneClientManager.clients_.insert(make_pair(event->clientPid, client));
    audioZoneClientManager.DispatchEvent(event);
}

void AudioZoneClientManagerSendZoneAddEventFuzzTest()
{
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientFuzzTest();
    AudioZoneClientManager &audioZoneClientManager = AudioZoneClientManager::GetInstance();
    pid_t clientPid = GetData<pid_t>();
    audioZoneClientManager.clients_.clear();
    audioZoneClientManager.clients_.insert(make_pair(clientPid, client));
    std::shared_ptr<AudioZoneDescriptor> descriptor = std::make_shared<AudioZoneDescriptor>();
    audioZoneClientManager.SendZoneAddEvent(clientPid, descriptor);
}

void AudioZoneClientManagerSendZoneRemoveEventFuzzTest()
{
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientFuzzTest();
    AudioZoneClientManager &audioZoneClientManager = AudioZoneClientManager::GetInstance();
    pid_t clientPid = GetData<pid_t>();
    int32_t zoneId = GetData<int32_t>();
    audioZoneClientManager.clients_.clear();
    audioZoneClientManager.clients_.insert(make_pair(clientPid, client));
    audioZoneClientManager.SendZoneRemoveEvent(clientPid, zoneId);
}

void AudioZoneClientManagerSendZoneChangeEventFuzzTest()
{
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientFuzzTest();
    AudioZoneClientManager &audioZoneClientManager = AudioZoneClientManager::GetInstance();
    pid_t clientPid = GetData<pid_t>();
    std::shared_ptr<AudioZoneDescriptor> descriptor = std::make_shared<AudioZoneDescriptor>();
    AudioZoneChangeReason reason = AudioZoneChangeReason::BIND_NEW_DEVICE;
    audioZoneClientManager.clients_.clear();
    audioZoneClientManager.clients_.insert(make_pair(clientPid, client));
    audioZoneClientManager.SendZoneChangeEvent(clientPid, descriptor, reason);
}

void AudioZoneClientManagerSendZoneInterruptEventFuzzTest()
{
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientFuzzTest();
    AudioZoneClientManager &audioZoneClientManager = AudioZoneClientManager::GetInstance();
    pid_t clientPid = GetData<pid_t>();
    std::shared_ptr<AudioZoneDescriptor> descriptor = std::make_shared<AudioZoneDescriptor>();
    AudioZoneInterruptReason reason = AudioZoneInterruptReason::LOCAL_INTERRUPT;
    int32_t zoneId = GetData<int32_t>();
    std::string deviceTag = "testDeviceTag";
    AudioInterrupt interrupt;
    AudioFocuState focusState = ACTIVE;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    interrupts.push_back(make_pair(interrupt, focusState));
    audioZoneClientManager.clients_.clear();
    audioZoneClientManager.clients_.insert(make_pair(clientPid, client));
    audioZoneClientManager.SendZoneInterruptEvent(clientPid, zoneId, deviceTag, interrupts, reason);
}

void AudioZoneClientManagerSetSystemVolumeLevelFuzzTest()
{
    if (g_testAudioStreamTypes.size() == 0) {
        return;
    }
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientFuzzTest();
    AudioZoneClientManager &audioZoneClientManager = AudioZoneClientManager::GetInstance();
    pid_t clientPid = GetData<pid_t>();
    int32_t zoneId = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    int32_t volumeFlag = GetData<int32_t>();
    AudioVolumeType volumeType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    audioZoneClientManager.clients_.clear();
    audioZoneClientManager.clients_.insert(make_pair(clientPid, client));
    audioZoneClientManager.SetSystemVolumeLevel(clientPid, zoneId, volumeType, volumeLevel, volumeFlag);
}

void AudioZoneClientManagerGetSystemVolumeLevelFuzzTest()
{
    if (g_testAudioStreamTypes.size() == 0) {
        return;
    }
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientFuzzTest();
    AudioZoneClientManager &audioZoneClientManager = AudioZoneClientManager::GetInstance();
    pid_t clientPid = GetData<pid_t>();
    int32_t zoneId = GetData<int32_t>();
    AudioVolumeType volumeType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    audioZoneClientManager.clients_.clear();
    audioZoneClientManager.clients_.insert(make_pair(clientPid, client));
    audioZoneClientManager.GetSystemVolumeLevel(clientPid, zoneId, volumeType);
}

void AudioZoneInterruptReporterEnableInterruptReportFuzzTest()
{
    AudioZoneInterruptReporter audioZoneInterruptReporter;
    pid_t clientPid = GetData<pid_t>();
    int32_t zoneId = GetData<int32_t>();
    bool enable = GetData<bool>();
    std::string deviceTag = "testDeviceTag";
    AudioZoneInterruptReporter::ReportItemList reportItemList;
    AudioZoneInterruptReporter::ReportItem item = make_pair(zoneId, deviceTag);
    reportItemList.push_back(item);
    audioZoneInterruptReporter.interruptEnableMaps_.insert(make_pair(clientPid, reportItemList));
    bool isNull = GetData<bool>();
    if (isNull) {
        audioZoneInterruptReporter.interruptEnableMaps_.clear();
    }
    audioZoneInterruptReporter.EnableInterruptReport(clientPid, zoneId, deviceTag, enable);
}

void AudioZoneInterruptReporterDisableInterruptReportFuzzTest()
{
    AudioZoneInterruptReporter audioZoneInterruptReporter;
    pid_t clientPid = GetData<pid_t>();
    int32_t zoneId = GetData<int32_t>();
    std::string deviceTag = "testDeviceTag";
    AudioZoneInterruptReporter::ReportItemList reportItemList;
    AudioZoneInterruptReporter::ReportItem item = make_pair(zoneId, deviceTag);
    reportItemList.push_back(item);
    audioZoneInterruptReporter.interruptEnableMaps_.insert(make_pair(clientPid, reportItemList));
    audioZoneInterruptReporter.DisableInterruptReport(clientPid);
    audioZoneInterruptReporter.DisableAllInterruptReport();
}

void AudioZoneInterruptReporterCreateReporterFuzzTest()
{
    AudioZoneInterruptReporter audioZoneInterruptReporter;
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::shared_ptr<AudioZoneClientManager> zoneClientManager = std::make_shared<AudioZoneClientManager>(nullptr);
    AudioZoneInterruptReason reason = AudioZoneInterruptReason::UNBIND_APP_FROM_ZONE;
    pid_t clientPid = GetData<pid_t>();
    int32_t zoneId = GetData<int32_t>();
    std::string deviceTag = "testDeviceTag";
    AudioZoneInterruptReporter::ReportItemList reportItemList;
    AudioZoneInterruptReporter::ReportItem item = make_pair(zoneId, deviceTag);
    reportItemList.push_back(item);
    audioZoneInterruptReporter.interruptEnableMaps_.insert(make_pair(clientPid, reportItemList));
    audioZoneInterruptReporter.CreateReporter(interruptService, zoneClientManager, reason);
}

void AudioZoneInterruptReporterGetFocusListFuzzTest()
{
    AudioZoneInterruptReporter audioZoneInterruptReporter;
    audioZoneInterruptReporter.interruptService_ = std::make_shared<AudioInterruptService>();
    audioZoneInterruptReporter.deviceTag_ = "testDeviceTag";
    bool isClear = GetData<bool>();
    if (isClear) {
        audioZoneInterruptReporter.deviceTag_.clear();
    }
    audioZoneInterruptReporter.GetFocusList();
}

void AudioZoneInterruptReporterReportInterruptFuzzTest()
{
    AudioZoneInterruptReporter audioZoneInterruptReporter;
    audioZoneInterruptReporter.interruptService_ = std::make_shared<AudioInterruptService>();
    audioZoneInterruptReporter.zoneClientManager_ = std::make_shared<AudioZoneClientManager>(nullptr);
    AudioInterrupt audioInterrupt;
    AudioFocuState focusState = ACTIVE;
    audioZoneInterruptReporter.oldFocusList_.push_back(make_pair(audioInterrupt, focusState));
    audioZoneInterruptReporter.ReportInterrupt();
}

TestPtr g_testPtrs[] = {
    AudioZoneServiceCreateAudioZoneFuzzTest,
    AudioZoneServiceReleaseAudioZoneFuzzTest,
    AudioZoneServiceGetAllAudioZoneFuzzTest,
    AudioZoneServiceBindDeviceToAudioZoneFuzzTest,
    AudioZoneServiceRemoveDeviceFromGlobalFuzzTest,
    AudioZoneServiceUnBindDeviceToAudioZoneFuzzTest,
    AudioZoneServiceRegisterAudioZoneClientFuzzTest,
    AudioZoneServiceUnRegisterAudioZoneClientFuzzTest,
    AudioZoneServiceInjectInterruptToAudioZoneFuzzTest,
    AudioZoneServiceFetchOutputDevicesFuzzTest,
    AudioZoneServiceFetchInputDeviceFuzzTest,
    AudioZoneServiceGetZoneStringDescriptorFuzzTest,
    AudioZoneServiceUpdateDeviceFromGlobalForAllZoneFuzzTest,
    AudioZoneServiceClearAudioFocusBySessionIDFuzzTest,
    AudioZoneClientManagerRegisterAudioZoneClientFuzzTest,
    AudioZoneClientManagerUnRegisterAudioZoneClientFuzzTest,
    AudioZoneClientManagerDispatchEventFuzzTest,
    AudioZoneClientManagerSendZoneAddEventFuzzTest,
    AudioZoneClientManagerSendZoneRemoveEventFuzzTest,
    AudioZoneClientManagerSendZoneChangeEventFuzzTest,
    AudioZoneClientManagerSendZoneInterruptEventFuzzTest,
    AudioZoneClientManagerSetSystemVolumeLevelFuzzTest,
    AudioZoneClientManagerGetSystemVolumeLevelFuzzTest,
    AudioZoneInterruptReporterEnableInterruptReportFuzzTest,
    AudioZoneInterruptReporterDisableInterruptReportFuzzTest,
    AudioZoneInterruptReporterCreateReporterFuzzTest,
    AudioZoneInterruptReporterGetFocusListFuzzTest,
    AudioZoneInterruptReporterReportInterruptFuzzTest,
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