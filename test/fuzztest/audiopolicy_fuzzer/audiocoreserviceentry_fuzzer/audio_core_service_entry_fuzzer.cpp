/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
shared_ptr<AudioCoreService> audioCoreService;
shared_ptr<AudioCoreService::EventEntry> eventEntry;
typedef void (*TestFuncs)();

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

template<typename T>
const T& PickValue(FuzzedDataProvider& fdp, const std::vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
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

void UpdateSessionOperationFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    constexpr int32_t operationCount = static_cast<int32_t>(SessionOperation::SESSION_OPERATION_RELEASE) + 1;
    SessionOperation operation =
        static_cast<SessionOperation>(fdp.ConsumeIntegralInRange<int32_t>(0, operationCount - 1));
    eventEntry->UpdateSessionOperation(sessionId, operation);
}

void OnServiceDisconnectedFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t serviceIndexCount = static_cast<int32_t>(AudioServiceIndex::AUDIO_SERVICE_INDEX) + 1;
    AudioServiceIndex serviceIndex =
        static_cast<AudioServiceIndex>(fdp.ConsumeIntegralInRange<int32_t>(0, serviceIndexCount - 1));
    eventEntry->OnServiceDisconnected(serviceIndex);
}

void OnServiceConnectedFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t serviceIndexCount = static_cast<int32_t>(AudioServiceIndex::AUDIO_SERVICE_INDEX) + 1;
    AudioServiceIndex serviceIndex =
        static_cast<AudioServiceIndex>(fdp.ConsumeIntegralInRange<int32_t>(0, serviceIndexCount - 1));
    eventEntry->OnServiceConnected(serviceIndex);
}

void CreateCapturerClientFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    uint32_t audioFlag = fdp.ConsumeIntegral<uint32_t>();
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    eventEntry->CreateCapturerClient(streamDesc, audioFlag, sessionId);
}

void SetDefaultOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    uint32_t sessionID = fdp.ConsumeIntegral<uint32_t>();
    StreamUsage streamUsage = PickValue(fdp, StreamUsageVec);
    bool isRunning = fdp.ConsumeBool();
    eventEntry->SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
}

void CreateRendererClientFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    uint32_t audioFlag = fdp.ConsumeIntegral<uint32_t>();
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    eventEntry->CreateRendererClient(streamDesc, audioFlag, sessionId, networkId);
}

void GetProcessDeviceInfoBySessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    AudioDeviceDescriptor deviceInfo;
    AudioStreamInfo info;
    bool isUltraFast = false;
    auto ret = eventEntry->GetProcessDeviceInfoBySessionId(sessionId, deviceInfo, info, isUltraFast);
}

void GetModuleNameBySessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    eventEntry->GetModuleNameBySessionId(sessionId);
}

void GenerateSessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto ret = eventEntry->GenerateSessionId();
}

void SetAudioSceneFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t audioSceneStart = static_cast<int32_t>(AudioScene::AUDIO_SCENE_INVALID);
    int32_t audioSceneEnd = static_cast<int32_t>(AudioScene::AUDIO_SCENE_MAX);
    AudioScene audioScene = static_cast<AudioScene>(fdp.ConsumeIntegralInRange<int32_t>(audioSceneStart,
        audioSceneEnd));
    eventEntry->SetAudioScene(audioScene);
}

void OnDeviceInfoUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor desc;
    desc.isEnable_ = fdp.ConsumeBool();
    int32_t commandStart = static_cast<int32_t>(DeviceInfoUpdateCommand::CATEGORY_UPDATE);
    int32_t commandEnd = static_cast<int32_t>(DeviceInfoUpdateCommand::EXCEPTION_FLAG_UPDATE);
    DeviceInfoUpdateCommand command =
        static_cast<DeviceInfoUpdateCommand>(fdp.ConsumeIntegralInRange<int32_t>(commandStart, commandEnd));
    eventEntry->OnDeviceInfoUpdated(desc, command);
}

void OnDeviceStatusUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor desc;
    bool isConnected = fdp.ConsumeBool();
    eventEntry->OnDeviceStatusUpdated(desc, isConnected);
}

void OnPnpDeviceStatusUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor desc;
    desc.deviceType_ = PickValue(fdp, DeviceTypeVec);
    bool isConnected = fdp.ConsumeBool();
    eventEntry->OnPnpDeviceStatusUpdated(desc, isConnected);
}

void AudioCoreServiceEventEntryLoadSplitModuleFuzzTest(FuzzedDataProvider& fdp)
{
    std::string splitArgs = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    eventEntry->LoadSplitModule(splitArgs, networkId);
}

void OnMicrophoneBlockedUpdateFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    int32_t statusCount = static_cast<int32_t>(DeviceBlockStatus::DEVICE_BLOCKED) + 1;
    DeviceBlockStatus status =
        static_cast<DeviceBlockStatus>(fdp.ConsumeIntegralInRange<int32_t>(0, statusCount - 1));
    eventEntry->OnMicrophoneBlockedUpdate(deviceType, status);
}


void AudioCoreServiceEventEntryReloadCaptureSessionFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    constexpr int32_t operationCount = static_cast<int32_t>(SessionOperation::SESSION_OPERATION_RELEASE) + 1;
    SessionOperation operation =
        static_cast<SessionOperation>(fdp.ConsumeIntegralInRange<int32_t>(0, operationCount - 1));
    eventEntry->ReloadCaptureSession(sessionId, operation);
}

void AudioCoreServiceEventEntryIsArmUsbDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.deviceType_ = PickValue(fdp, DeviceTypeVec);
    eventEntry->IsArmUsbDevice(deviceDesc);
}

void AudioCoreServiceEventEntryOnDeviceConfigurationChangedFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    std::string macAddress = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string deviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    AudioStreamInfo streamInfo;
    eventEntry->OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

void AudioCoreServiceEventEntryOnForcedDeviceSelectedFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType devType = PickValue(fdp, DeviceTypeVec);
    std::string macAddress = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    eventEntry->OnPrivacyDeviceSelected(devType, macAddress);
    eventEntry->OnForcedDeviceSelected(devType, macAddress);
     
    auto &devMan = AudioDeviceManager::GetAudioDeviceManager();
    AudioDeviceStatus::GetInstance().OnPrivacyDeviceSelected(devType, macAddress);
    auto devDesc = make_shared<AudioDeviceDescriptor>();
    devDesc->deviceId_ = fdp.ConsumeIntegral<uint32_t>();
    devDesc->deviceType_ = devType;
    devDesc->macAddress_ = macAddress;
    devDesc->deviceRole_ = OUTPUT_DEVICE;
    devMan.AddNewDevice(devDesc);
    auto devDesc2 = make_shared<AudioDeviceDescriptor>();
    devDesc2->deviceId_ = fdp.ConsumeIntegral<uint32_t>();
    devDesc2->deviceType_ = devType;
    devDesc2->macAddress_ = macAddress;
    devDesc2->deviceRole_ = INPUT_DEVICE;
    devMan.AddNewDevice(devDesc2);
    AudioDeviceStatus::GetInstance().OnPrivacyDeviceSelected(devType, macAddress);
}

void AudioCoreServiceEventEntryGetDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceFlag deviceFlag = ConsumeEnum<DeviceFlag>(fdp);
    eventEntry->GetDevices(deviceFlag);
}

void AudioCoreServiceEventEntryGetPreferredOutputDeviceDescriptorsFuzzTest(FuzzedDataProvider& fdp)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.contentType = CONTENT_TYPE_UNKNOWN;
    rendererInfo.streamUsage = PickValue(fdp, StreamUsageVec);
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    eventEntry->GetPreferredOutputDeviceDescriptors(rendererInfo, uid, networkId);
}

void AudioCoreServiceEventEntrySetDeviceActiveFuzzTest(FuzzedDataProvider& fdp)
{
    InternalDeviceType deviceType = ConsumeEnum<InternalDeviceType>(fdp);
    bool active = fdp.ConsumeBool();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    eventEntry->SetDeviceActive(deviceType, active, uid);
}

void AudioCoreServiceEventEntryGetActiveBluetoothDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    eventEntry->GetActiveBluetoothDevice();
}

void AudioCoreServiceEventEntrySetCallDeviceActiveFuzzTest(FuzzedDataProvider& fdp)
{
    InternalDeviceType deviceType = ConsumeEnum<InternalDeviceType>(fdp);
    bool active = fdp.ConsumeBool();
    std::string address = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    eventEntry->SetCallDeviceActive(deviceType, active, address, uid);
}

void AudioCoreServiceEventEntryGetPreferredInputDeviceDescriptorsFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = ConsumeEnum<SourceType>(fdp);
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    eventEntry->GetPreferredInputDeviceDescriptors(capturerInfo, uid, networkId);
}

void AudioCoreServiceEventEntryUpdateTrackerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioMode mode = ConsumeEnum<AudioMode>(fdp);
    AudioStreamChangeInfo streamChangeInfo;
    eventEntry->UpdateTracker(mode, streamChangeInfo);
}

void AudioCoreServiceEventEntryRegisteredTrackerClientDiedFuzzTest(FuzzedDataProvider& fdp)
{
    pid_t uid = static_cast<pid_t>(fdp.ConsumeIntegral<int32_t>());
    pid_t pid = static_cast<pid_t>(fdp.ConsumeIntegral<int32_t>());
    eventEntry->RegisteredTrackerClientDied(uid, pid);
}

void AudioCoreServiceEventEntryRegisterTrackerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioMode mode = ConsumeEnum<AudioMode>(fdp);
    AudioStreamChangeInfo streamChangeInfo;
    sptr<IRemoteObject> object = nullptr;
    int32_t apiVersion = fdp.ConsumeIntegral<int32_t>();
    eventEntry->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
}

void AudioCoreServiceEventEntryGetAvailableMicrophonesFuzzTest(FuzzedDataProvider& fdp)
{
    eventEntry->GetAvailableMicrophones();
}

void AudioCoreServiceEventEntryOnReceiveUpdateDeviceNameEventFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string deviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    eventEntry->OnReceiveUpdateDeviceNameEvent(macAddress, deviceName);
}

void AudioCoreServiceEventEntryGetAudioCapturerMicrophoneDescriptorsFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    eventEntry->GetAudioCapturerMicrophoneDescriptors(sessionId);
}

void AudioCoreServiceEventEntrySelectInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = PickValue(fdp, DeviceTypeVec);
    audioDevDesc->networkId_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    selectedDesc.push_back(audioDevDesc);
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = fdp.ConsumeIntegral<int32_t>();
    audioCoreService->Init();
    eventEntry->SelectInputDevice(audioCapturerFilter, selectedDesc);
}

void AudioCoreServiceEventEntryGetCurrentRendererChangeInfosFuzzTest(FuzzedDataProvider& fdp)
{
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    bool hasBTPermission = fdp.ConsumeBool();
    bool hasSystemPermission = fdp.ConsumeBool();
    eventEntry->GetCurrentRendererChangeInfos(audioRendererChangeInfos, hasBTPermission, hasSystemPermission);
}

void AudioCoreServiceEventEntrySelectOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    audioRendererFilter->uid = fdp.ConsumeIntegral<int32_t>();
    audioRendererFilter->rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    audioRendererFilter->rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioRendererFilter->rendererInfo.rendererFlags = fdp.ConsumeIntegral<int32_t>();
    audioRendererFilter->streamId = fdp.ConsumeIntegral<uint32_t>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = PickValue(fdp, DeviceTypeVec);
    audioDevDesc->networkId_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    selectedDesc.push_back(audioDevDesc);
    eventEntry->SelectOutputDevice(audioRendererFilter, selectedDesc);
}


void AudioCoreServiceEventEntryGetCurrentCapturerChangeInfosFuzzTest(FuzzedDataProvider& fdp)
{
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    bool hasBTPermission = fdp.ConsumeBool();
    bool hasSystemPermission = fdp.ConsumeBool();
    eventEntry->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos, hasBTPermission, hasSystemPermission);
}

void AudioCoreServiceEventEntryOnCapturerSessionAddedFuzzTest(FuzzedDataProvider& fdp)
{
    uint64_t sessionID = fdp.ConsumeIntegral<uint64_t>();
    SessionInfo sessionInfo;
    sessionInfo.sourceType = ConsumeEnum<SourceType>(fdp);
    sessionInfo.rate = fdp.ConsumeIntegralInRange<uint32_t>(0, 1);
    sessionInfo.channels = fdp.ConsumeIntegralInRange<uint32_t>(0, 1);
    AudioStreamInfo streamInfo;
    eventEntry->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
}

void AudioCoreServiceEventEntryNotifyRemoteRenderStateFuzzTest(FuzzedDataProvider& fdp)
{
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string condition = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    eventEntry->NotifyRemoteRenderState(networkId, condition, value);
}

void AudioCoreServiceEventEntryOnCapturerSessionRemovedFuzzTest(FuzzedDataProvider& fdp)
{
    uint64_t sessionID = fdp.ConsumeIntegral<uint64_t>();
    eventEntry->OnCapturerSessionRemoved(sessionID);
}

void AudioCoreServiceEventEntryGetVolumeGroupInfosFuzzTest(FuzzedDataProvider& fdp)
{
    eventEntry->GetVolumeGroupInfos();
}

void AudioCoreServiceEventEntryTriggerFetchDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp);
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->Init();
    eventEntry->TriggerFetchDevice(reason);
}

void AudioCoreServiceEventEntryExcludeOutputDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage audioDevUsage = ConsumeEnum<AudioDeviceUsage>(fdp);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = PickValue(fdp, DeviceTypeVec);
    audioDevDesc->networkId_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioDeviceDescriptors.push_back(audioDevDesc);
    audioCoreService->Init();
    eventEntry->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

void AudioCoreServiceEventEntryGetExcludedDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage audioDevUsage = ConsumeEnum<AudioDeviceUsage>(fdp);
    eventEntry->GetExcludedDevices(audioDevUsage);
}

void AudioCoreServiceEventEntryUnexcludeOutputDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage audioDevUsage = ConsumeEnum<AudioDeviceUsage>(fdp);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = PickValue(fdp, DeviceTypeVec);
    audioDevDesc->networkId_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioDeviceDescriptors.push_back(audioDevDesc);
    audioCoreService->Init();
    eventEntry->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

void AudioCoreServiceEventEntrySetSessionDefaultOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    int32_t callerPid = fdp.ConsumeIntegral<int32_t>();
    eventEntry->SetSessionDefaultOutputDevice(callerPid, deviceType);
}

void AudioCoreServiceEventEntryGetSessionDefaultOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    int32_t callerPid = fdp.ConsumeIntegral<int32_t>();
    eventEntry->GetSessionDefaultOutputDevice(callerPid, deviceType);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    UpdateSessionOperationFuzzTest,
    OnServiceConnectedFuzzTest,
    OnServiceDisconnectedFuzzTest,
    CreateRendererClientFuzzTest,
    CreateCapturerClientFuzzTest,
    SetDefaultOutputDeviceFuzzTest,
    GetModuleNameBySessionIdFuzzTest,
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
    AudioCoreServiceEventEntryExcludeOutputDevicesFuzzTest,
    AudioCoreServiceEventEntryUnexcludeOutputDevicesFuzzTest,
    AudioCoreServiceEventEntryGetExcludedDevicesFuzzTest,
    AudioCoreServiceEventEntrySetSessionDefaultOutputDeviceFuzzTest,
    AudioCoreServiceEventEntryGetSessionDefaultOutputDeviceFuzzTest,
    });
    func(fdp);
}
void Init()
{
    audioCoreService = std::make_shared<AudioCoreService>();
    eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
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
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
