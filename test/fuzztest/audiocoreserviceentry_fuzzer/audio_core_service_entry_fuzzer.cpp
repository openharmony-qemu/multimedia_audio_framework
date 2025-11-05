/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <cstring>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
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

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

vector<DeviceType> DeviceTypeVec = {
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
    DEVICE_TYPE_MAX,
};

vector<StreamUsage> StreamUsageVec = {
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

void UpdateSessionOperationFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint32_t sessionId = 0;
    constexpr int32_t operationCount = static_cast<int32_t>(SessionOperation::SESSION_OPERATION_RELEASE) + 1;
    SessionOperation operation = static_cast<SessionOperation>(GetData<uint8_t>() % operationCount);
    eventEntry->UpdateSessionOperation(sessionId, operation);
}

void OnServiceConnectedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    int32_t serviceIndexCount = static_cast<int32_t>(AudioServiceIndex::AUDIO_SERVICE_INDEX) + 1;
    AudioServiceIndex serviceIndex = static_cast<AudioServiceIndex>(GetData<uint8_t>() % serviceIndexCount);
    eventEntry->OnServiceConnected(serviceIndex);
}

void OnServiceDisconnectedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    int32_t serviceIndexCount = static_cast<int32_t>(AudioServiceIndex::AUDIO_SERVICE_INDEX) + 1;
    AudioServiceIndex serviceIndex = static_cast<AudioServiceIndex>(GetData<uint8_t>() % serviceIndexCount);
    eventEntry->OnServiceDisconnected(serviceIndex);
}

void CreateRendererClientFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    uint32_t audioFlag = 0;
    uint32_t sessionId = 0;
    std::string networkId = "";
    eventEntry->CreateRendererClient(streamDesc, audioFlag, sessionId, networkId);
}

void CreateCapturerClientFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    uint32_t audioFlag = 0;
    uint32_t sessionId = 0;
    eventEntry->CreateCapturerClient(streamDesc, audioFlag, sessionId);
}

void SetDefaultOutputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    uint32_t sessionID = 0;
    uint32_t streamUsageCount = GetData<uint32_t>() % StreamUsageVec.size();
    StreamUsage streamUsage = StreamUsageVec[streamUsageCount];
    bool isRunning = true;
    eventEntry->SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
}

void GetAdapterNameBySessionIdFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint32_t sessionId = 0;
    eventEntry->GetAdapterNameBySessionId(sessionId);
}

void GetProcessDeviceInfoBySessionIdFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint32_t sessionId = 0;
    AudioDeviceDescriptor deviceInfo;
    AudioStreamInfo info;
    auto ret = eventEntry->GetProcessDeviceInfoBySessionId(sessionId, deviceInfo, info);
}

void GenerateSessionIdFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    auto ret = eventEntry->GenerateSessionId();
}

void OnDeviceInfoUpdatedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioDeviceDescriptor desc;
    desc.isEnable_ = true;
    int32_t commandCount = static_cast<int32_t>(DeviceInfoUpdateCommand::EXCEPTION_FLAG_UPDATE
                                                - DeviceInfoUpdateCommand::CATEGORY_UPDATE) + 1;
    DeviceInfoUpdateCommand command = static_cast<DeviceInfoUpdateCommand>(GetData<uint8_t>() % commandCount + 1);
    eventEntry->OnDeviceInfoUpdated(desc, command);
}

void SetAudioSceneFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    int32_t audioSceneCount = static_cast<int32_t>(AudioScene::AUDIO_SCENE_MAX - AudioScene::AUDIO_SCENE_INVALID) + 1;
    AudioScene audioScene = static_cast<AudioScene>(GetData<uint8_t>() % audioSceneCount - 1);
    eventEntry->SetAudioScene(audioScene);
}

void OnDeviceStatusUpdatedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioDeviceDescriptor desc;
    bool isConnected = GetData<uint32_t>() % NUM_2;
    eventEntry->OnDeviceStatusUpdated(desc, isConnected);
}

void OnMicrophoneBlockedUpdateFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    int32_t statusCount = static_cast<int32_t>(DeviceBlockStatus::DEVICE_BLOCKED) + 1;
    DeviceBlockStatus status = static_cast<DeviceBlockStatus>(GetData<uint8_t>() % statusCount);
    eventEntry->OnMicrophoneBlockedUpdate(deviceType, status);
}

void OnPnpDeviceStatusUpdatedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioDeviceDescriptor desc;
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc.deviceType_ = DeviceTypeVec[deviceTypeCount];
    bool isConnected = GetData<uint32_t>() % NUM_2;
    eventEntry->OnPnpDeviceStatusUpdated(desc, isConnected);
}

void AudioCoreServiceEventEntryReloadCaptureSessionFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint32_t sessionId = 0;
    constexpr int32_t operationCount = static_cast<int32_t>(SessionOperation::SESSION_OPERATION_RELEASE) + 1;
    SessionOperation operation = static_cast<SessionOperation>(GetData<uint8_t>() % operationCount);
    eventEntry->ReloadCaptureSession(sessionId, operation);
}

void AudioCoreServiceEventEntryLoadSplitModuleFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    std::string splitArgs = "splitArgs";
    std::string networkId = "networkId";
    eventEntry->LoadSplitModule(splitArgs, networkId);
}

void AudioCoreServiceEventEntryOnDeviceConfigurationChangedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    DeviceType deviceType = DeviceType::DEVICE_TYPE_USB_HEADSET;
    std::string macAddress = "11-22-33-44-55-66";
    std::string deviceName = "deviceName";
    AudioStreamInfo streamInfo;
    eventEntry->OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

void AudioCoreServiceEventEntryOnForcedDeviceSelectedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    DeviceType devType = DeviceType::DEVICE_TYPE_USB_HEADSET;
    std::string macAddress = "11-22-33-44-55-66";
    eventEntry->OnPrivacyDeviceSelected(devType, macAddress);
    eventEntry->OnForcedDeviceSelected(devType, macAddress);
     
    auto &devMan = AudioDeviceManager::GetAudioDeviceManager();
    AudioDeviceStatus::GetInstance().OnPrivacyDeviceSelected(devType, macAddress);
    auto devDesc = make_shared<AudioDeviceDescriptor>();
    devDesc->deviceId_ = GetData<uint32_t>();
    devDesc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    devDesc->macAddress_ = macAddress;
    devDesc->deviceRole_ = OUTPUT_DEVICE;
    devMan.AddNewDevice(devDesc);
    auto devDesc2 = make_shared<AudioDeviceDescriptor>();
    devDesc2->deviceId_ = GetData<uint32_t>();
    devDesc2->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    devDesc2->macAddress_ = macAddress;
    devDesc2->deviceRole_ = INPUT_DEVICE;
    devMan.AddNewDevice(devDesc2);
    AudioDeviceStatus::GetInstance().OnPrivacyDeviceSelected(devType, macAddress);
}

void AudioCoreServiceEventEntryIsArmUsbDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioDeviceDescriptor deviceDesc;
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    deviceDesc.deviceType_ = DeviceTypeVec[deviceTypeCount];
    eventEntry->IsArmUsbDevice(deviceDesc);
}

void AudioCoreServiceEventEntryGetDevicesFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    DeviceFlag deviceFlag = DeviceFlag::NONE_DEVICES_FLAG;
    eventEntry->GetDevices(deviceFlag);
}

void AudioCoreServiceEventEntrySetDeviceActiveFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    InternalDeviceType deviceType = DEVICE_TYPE_EARPIECE;
    bool active = true;
    int32_t uid = GetData<uint32_t>() % NUM_2;
    eventEntry->SetDeviceActive(deviceType, active, uid);
}

void AudioCoreServiceEventEntryGetPreferredOutputDeviceDescriptorsFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioRendererInfo rendererInfo;
    rendererInfo.contentType = CONTENT_TYPE_UNKNOWN;
    rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    std::string networkId = "networkId";
    eventEntry->GetPreferredOutputDeviceDescriptors(rendererInfo, networkId);
}

void AudioCoreServiceEventEntryGetPreferredInputDeviceDescriptorsFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = SOURCE_TYPE_INVALID;
    std::string networkId = "networkId";
    eventEntry->GetPreferredInputDeviceDescriptors(capturerInfo, INVALID_UID, networkId);
}

void AudioCoreServiceEventEntryGetActiveBluetoothDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    eventEntry->GetActiveBluetoothDevice();
}

void AudioCoreServiceEventEntrySetCallDeviceActiveFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    InternalDeviceType deviceType = DEVICE_TYPE_EARPIECE;
    bool active = true;
    std::string address = "11-22-33-44-55-66";
    int32_t uid = GetData<uint32_t>() % NUM_2;
    eventEntry->SetCallDeviceActive(deviceType, active, address, uid);
}

void AudioCoreServiceEventEntryRegisterTrackerFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    sptr<IRemoteObject> object = nullptr;
    int32_t apiVersion = GetData<uint32_t>() % NUM_2;
    eventEntry->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
}

void AudioCoreServiceEventEntryUpdateTrackerFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    eventEntry->UpdateTracker(mode, streamChangeInfo);
}

void AudioCoreServiceEventEntryRegisteredTrackerClientDiedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    pid_t uid = 0;
    pid_t pid = 0;
    eventEntry->RegisteredTrackerClientDied(uid, pid);
}

void AudioCoreServiceEventEntryGetAvailableMicrophonesFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    eventEntry->GetAvailableMicrophones();
}

void AudioCoreServiceEventEntryGetAudioCapturerMicrophoneDescriptorsFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    int32_t sessionId = 0;
    eventEntry->GetAudioCapturerMicrophoneDescriptors(sessionId);
}

void AudioCoreServiceEventEntryOnReceiveUpdateDeviceNameEventFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    std::string macAddress = "11-22-33-44-55-66";
    std::string deviceName = "deviceName";
    eventEntry->OnReceiveUpdateDeviceNameEvent(macAddress, deviceName);
}

void AudioCoreServiceEventEntrySelectOutputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    audioRendererFilter->uid = GetData<uint32_t>() % NUM_2;
    audioRendererFilter->rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    audioRendererFilter->rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioRendererFilter->rendererInfo.rendererFlags = 0;
    audioRendererFilter->streamId = 0;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    selectedDesc.push_back(audioDevDesc);
    eventEntry->SelectOutputDevice(audioRendererFilter, selectedDesc);
}

void AudioCoreServiceEventEntrySelectInputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    selectedDesc.push_back(audioDevDesc);
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = GetData<uint32_t>() % NUM_2;
    audioCoreService->Init();
    eventEntry->SelectInputDevice(audioCapturerFilter, selectedDesc);
}

void AudioCoreServiceEventEntryGetCurrentRendererChangeInfosFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    bool hasBTPermission = true;
    bool hasSystemPermission = true;
    eventEntry->GetCurrentRendererChangeInfos(audioRendererChangeInfos, hasBTPermission, hasSystemPermission);
}

void AudioCoreServiceEventEntryGetCurrentCapturerChangeInfosFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    bool hasBTPermission = true;
    bool hasSystemPermission = true;
    eventEntry->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos, hasBTPermission, hasSystemPermission);
}

void AudioCoreServiceEventEntryNotifyRemoteRenderStateFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    std::string networkId = "LocalDevice";
    std::string condition = "";
    std::string value = "";
    eventEntry->NotifyRemoteRenderState(networkId, condition, value);
}

void AudioCoreServiceEventEntryOnCapturerSessionAddedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint64_t sessionID = 0;
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    sessionInfo.rate = GetData<uint32_t>() % NUM_2;
    sessionInfo.channels = GetData<uint32_t>() % NUM_2;
    AudioStreamInfo streamInfo;
    eventEntry->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
}

void AudioCoreServiceEventEntryOnCapturerSessionRemovedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint64_t sessionID = 0;
    eventEntry->OnCapturerSessionRemoved(sessionID);
}

void AudioCoreServiceEventEntryTriggerFetchDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->Init();
    eventEntry->TriggerFetchDevice(reason);
}

void AudioCoreServiceEventEntryGetVolumeGroupInfosFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    eventEntry->GetVolumeGroupInfos();
}

void AudioCoreServiceEventEntryFetchInputDeviceForTrackFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioStreamChangeInfo streamChangeInfo;
    eventEntry->FetchInputDeviceForTrack(streamChangeInfo);
}

void AudioCoreServiceEventEntryExcludeOutputDevicesFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);
    audioCoreService->Init();
    eventEntry->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

void AudioCoreServiceEventEntryUnexcludeOutputDevicesFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);
    audioCoreService->Init();
    eventEntry->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

void AudioCoreServiceEventEntryGetExcludedDevicesFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    eventEntry->GetExcludedDevices(audioDevUsage);
}

void AudioCoreServiceEventEntryGetPreferredOutputStreamTypeFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioRendererInfo rendererInfo;
    std::string bundleName = "bundleName";
    eventEntry->GetPreferredOutputStreamType(rendererInfo, bundleName);
}

void AudioCoreServiceEventEntrySetSessionDefaultOutputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    DeviceType deviceType = DeviceType::DEVICE_TYPE_USB_HEADSET;
    int32_t callerPid = 0;
    eventEntry->SetSessionDefaultOutputDevice(callerPid, deviceType);
}

void AudioCoreServiceEventEntryGetSessionDefaultOutputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    DeviceType deviceType = DeviceType::DEVICE_TYPE_USB_HEADSET;
    int32_t callerPid = 0;
    eventEntry->GetSessionDefaultOutputDevice(callerPid, deviceType);
}

void AudioCoreServiceEventEntryGetPreferredInputStreamTypeFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = SOURCE_TYPE_INVALID;
    eventEntry->GetPreferredInputStreamType(capturerInfo);
}

TestFuncs g_testFuncs[] = {
    UpdateSessionOperationFuzzTest,
    OnServiceConnectedFuzzTest,
    OnServiceDisconnectedFuzzTest,
    CreateRendererClientFuzzTest,
    CreateCapturerClientFuzzTest,
    SetDefaultOutputDeviceFuzzTest,
    GetAdapterNameBySessionIdFuzzTest,
    GetProcessDeviceInfoBySessionIdFuzzTest,
    GenerateSessionIdFuzzTest,
    OnDeviceInfoUpdatedFuzzTest,
    SetAudioSceneFuzzTest,
    OnDeviceStatusUpdatedFuzzTest,
    OnMicrophoneBlockedUpdateFuzzTest,
    OnPnpDeviceStatusUpdatedFuzzTest,
    AudioCoreServiceEventEntryReloadCaptureSessionFuzzTest,
    AudioCoreServiceEventEntryLoadSplitModuleFuzzTest,
    AudioCoreServiceEventEntryOnDeviceConfigurationChangedFuzzTest,
    AudioCoreServiceEventEntryOnForcedDeviceSelectedFuzzTest,
    AudioCoreServiceEventEntryIsArmUsbDeviceFuzzTest,
    AudioCoreServiceEventEntryGetDevicesFuzzTest,
    AudioCoreServiceEventEntrySetDeviceActiveFuzzTest,
    AudioCoreServiceEventEntryGetPreferredOutputDeviceDescriptorsFuzzTest,
    AudioCoreServiceEventEntryGetPreferredInputDeviceDescriptorsFuzzTest,
    AudioCoreServiceEventEntryGetActiveBluetoothDeviceFuzzTest,
    AudioCoreServiceEventEntrySetCallDeviceActiveFuzzTest,
    AudioCoreServiceEventEntryRegisterTrackerFuzzTest,
    AudioCoreServiceEventEntryUpdateTrackerFuzzTest,
    AudioCoreServiceEventEntryRegisteredTrackerClientDiedFuzzTest,
    AudioCoreServiceEventEntryGetAvailableMicrophonesFuzzTest,
    AudioCoreServiceEventEntryGetAudioCapturerMicrophoneDescriptorsFuzzTest,
    AudioCoreServiceEventEntryOnReceiveUpdateDeviceNameEventFuzzTest,
    AudioCoreServiceEventEntrySelectOutputDeviceFuzzTest,
    AudioCoreServiceEventEntrySelectInputDeviceFuzzTest,
    AudioCoreServiceEventEntryGetCurrentRendererChangeInfosFuzzTest,
    AudioCoreServiceEventEntryGetCurrentCapturerChangeInfosFuzzTest,
    AudioCoreServiceEventEntryNotifyRemoteRenderStateFuzzTest,
    AudioCoreServiceEventEntryOnCapturerSessionAddedFuzzTest,
    AudioCoreServiceEventEntryOnCapturerSessionRemovedFuzzTest,
    AudioCoreServiceEventEntryTriggerFetchDeviceFuzzTest,
    AudioCoreServiceEventEntryGetVolumeGroupInfosFuzzTest,
    AudioCoreServiceEventEntryFetchInputDeviceForTrackFuzzTest,
    AudioCoreServiceEventEntryExcludeOutputDevicesFuzzTest,
    AudioCoreServiceEventEntryUnexcludeOutputDevicesFuzzTest,
    AudioCoreServiceEventEntryGetExcludedDevicesFuzzTest,
    AudioCoreServiceEventEntryGetPreferredOutputStreamTypeFuzzTest,
    AudioCoreServiceEventEntrySetSessionDefaultOutputDeviceFuzzTest,
    AudioCoreServiceEventEntryGetSessionDefaultOutputDeviceFuzzTest,
    AudioCoreServiceEventEntryGetPreferredInputStreamTypeFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
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
